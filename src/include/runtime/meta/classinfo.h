#pragma once

#include <runtime/meta/attribute.h>
#include <runtime/meta/functioninfo.h>
#include <runtime/utils/typelist.h>
#include <runtime/utils/tupleutility.h>
#include <runtime/utils/preprocessor.h>

#include <EngineAssert.h>

#include <string_view>
#include <type_traits>

namespace Runtime {
namespace Meta {


/**
	Class name
*/
struct ClassNameAttribute : Attribute
{
	const char* const name;

	ClassNameAttribute(const char* name_): name(name_)
	{}
};


/**

*/
struct ClassBaseAttribute : Attribute
{};

/**

*/
template<typename ... T>
struct ClassBase : AttributeValue<ClassBaseAttribute> {
	using type = TypeList<T...>;
};

/**

*/
template<typename T, typename ... Attribs__>
class FieldInfo
{
public:

	using ValueType = std::remove_cv_t<T>;
	using Attributes = std::tuple<Attribs__...>;

	template<typename A>
	static constexpr inline bool HasAttribute = Tuple::template contains<Attributes, A>();

	FieldInfo(T* value_, std::string_view name_, Attribs__ ... attributes)
		: m_value(value_)
		, m_name(name_)
		, m_attributes{std::move(attributes)...}
	{}

	constexpr bool isConst() const
	{
		return std::is_const_v<T>;
	}

	T& value() const
	{
		Assert2(m_value != nullptr, "Attempt to access filed of uninitialized instance");
		return *m_value;
	}

	std::string_view name() const
	{
		return m_name;
	}

	decltype(auto) attributes() const
	{
		return (m_attributes);
	}

	template<typename Attribute>
	constexpr bool hasAttribute() const
	{
		return HasAttribute<Attribute>;
	}

private:

	T* const m_value;
	std::string_view m_name;
	const Attributes m_attributes;
};

template<typename T, typename ... A>
FieldInfo(T*, std::string_view, A...) -> FieldInfo<T, A...>;


/**

*/
struct ClassFieldsAttribute : Attribute
{};

/**

*/
template<typename F>
struct ClassFields : AttributeValue<ClassFieldsAttribute>
{
	F fieldsProvider;

	ClassFields(F fieldsProvider_): fieldsProvider(std::move(fieldsProvider_))
	{}

	template<typename T>
	decltype(auto) operator ()(T* instance) const {
		return fieldsProvider(instance);
	}
};



/// <summary>
///
/// </summary>
template<auto F, typename ... Attributes>
class MethodInfo
{
public:
	using FunctionType = decltype(F);
	using FunctionTypeInfo = Meta::FunctionTypeInfo<FunctionType>;
	using AttributesTuple = std::tuple<Attributes...>;

	inline constexpr static bool IsMemberFunction = std::is_member_function_pointer_v<FunctionType>; 

	inline constexpr static FunctionType Func = F;

	MethodInfo(std::string_view name_, Attributes ... attributes_)
		: m_name(name_)
		, m_attributes{std::move(attributes_)...}
	{}

	std::string_view name() const
	{
		return m_name;
	}

	const AttributesTuple& attributes() const
	{
		return m_attributes;
	}

private:

	std::string_view m_name;
	AttributesTuple m_attributes;
};


template<auto F, typename ... Attributes>
auto createMethodInfo(std::string_view name, Attributes&& ... attributes)
{
	return MethodInfo<F, Attributes...>{name, std::forward<Attributes>(attributes)...};
}



/// <summary>
///
/// </summary>
struct ClassMethodsAttribute final : Meta::Attribute
{};


/// <summary>
///
/// </summary>
template<typename ...M>
class ClassMethods final : public Meta::AttributeValue<ClassMethodsAttribute>
{
public:

	ClassMethods(M... methods_) : m_methods {std::move(methods_)...}
	{}

	const auto methods() const
	{
		return m_methods;
	}

private:

	std::tuple<M...> m_methods;
};


} // namespace Meta

//-----------------------------------------------------------------------------
namespace MetaDetail {

/**

*/
template<typename T, bool = attributeDefined<T, Meta::ClassBaseAttribute>()>
struct ClassDirectBase__
{
	using type = typename Meta::AttributeValueType<T, Meta::ClassBaseAttribute>::type;
};

/**

*/
template<typename T>
struct ClassDirectBase__<T, false>
{
	using type = TypeList<>;
};


template<typename T, typename Base = typename ClassDirectBase__<T>::type>
struct ClassAllBase__;


template<typename T, typename ... Bases>
struct ClassAllBase__<T, TypeList<Bases...>> {
	using type = typelist::Concat<TypeList<Bases...>, typename ClassAllBase__<Bases>::type...>;
};


template<typename T>
auto GetClassFields(T* instance) {
	
	using Type = std::remove_cv_t<T>;

	if constexpr (attributeDefined<Type, Meta::ClassFieldsAttribute>()) {
		auto fields =  Meta::attributeValue<Type, Meta::ClassFieldsAttribute>();
		return fields(instance);
	}
	else {
		return std::tuple{};
	}
}

template<bool IsConst, typename T, typename = typename ClassDirectBase__<std::remove_const_t<T>>::type>
struct ClassAndBaseFields;

template<typename T, typename ... Base>
struct ClassAndBaseFields<false, T, TypeList<Base...>>
{
	static auto GetFields(T* instance) {
		return std::tuple_cat(
			ClassAndBaseFields<false, Base>::GetFields(static_cast<Base*>(instance)) ... ,
			GetClassFields(instance)
		);
	}
};

template<typename T, typename ... Base>
struct ClassAndBaseFields<true, T, TypeList<Base...>>
{
	static auto GetFields(const T* instance) {
		return std::tuple_cat(
			ClassAndBaseFields<true, Base>::GetFields(static_cast<const Base*>(instance)) ... ,
			GetClassFields(instance)
		);
	}
};

#pragma region Class Meta methods access

template<typename T>
auto classMethods__() {
	using Type = std::remove_cv_t<T>;

	if constexpr (!Meta::AttributeDefined<Type, Meta::ClassMethodsAttribute>)
	{
		return std::tuple{};
	}
	else
	{
		return Meta::attributeValue<Type, Meta::ClassMethodsAttribute>().methods();
	}
}

template<typename T, typename = typename ClassDirectBase__<std::remove_const_t<T>>::type>
struct ClassAndBaseMethods;

template<typename T, typename ... Base>
struct ClassAndBaseMethods<T, TypeList<Base...>>
{
	static auto methods()
	{
		return std::tuple_cat(
			ClassAndBaseMethods<Base>::methods() ...,
			classMethods__<T>()
		);
	}
};

#pragma endregion



} // namespace MetaDetail

//-----------------------------------------------------------------------------
namespace Meta {

template<typename T>
using ClassDirectBase = typename Runtime::MetaDetail::ClassDirectBase__<T>::type;

template<typename T>
using ClassAllBase = typename Runtime::MetaDetail::ClassAllBase__<T>::type;

template<typename T>
using ClassAllUniqueBase = typelist::Distinct<ClassAllBase<T>>;

template<typename T>
[[nodiscard]] auto classFields(T& instance)
{
	return Runtime::MetaDetail::ClassAndBaseFields<false, T>::GetFields(&instance);
}

template<typename T>
[[nodiscard]] auto classFields(const T& instance)
{
	return Runtime::MetaDetail::ClassAndBaseFields<true, T>::GetFields(&instance);
}

template<typename T>
[[nodiscard]] auto classFields()
{
	const T* noInstance = nullptr;
	return Runtime::MetaDetail::ClassAndBaseFields<true, T>::GetFields(noInstance);
}

template<typename T>
[[nodiscard]] decltype(auto) classMethods()
{
	return Runtime::MetaDetail::ClassAndBaseMethods<T>::methods();
}

namespace MetaDetail 
{

template<auto Target, typename F>
constexpr bool sameMethod()
{
	if constexpr (std::is_convertible_v<typename F::FunctionType, decltype(Target)>)
	{
		return F::Func == Target;
	}

	return false;
}


template<auto Target, int Idx, typename>
struct MethodInfoFinder;


template<auto Target, int Idx, typename Head, typename ... Tail>
struct MethodInfoFinder<Target, Idx, std::tuple<Head, Tail...>>
{
	static constexpr inline int result = sameMethod<Target, Head>() ? Idx : MethodInfoFinder<Target, Idx + 1, std::tuple<Tail...>>::result;
};

template<auto Target, int Idx>
struct MethodInfoFinder<Target, Idx, std::tuple<>>
{
	static constexpr inline int result = -1;
};

} // namespace MetaDetail 


template<auto Func>
static auto findClassMethodInfo()
{
	using FuncType = decltype(Func);

	static_assert(IsMemberFunction<FuncType>, "Only member function can be used");

	using FuncInfo = FunctionTypeInfo<FuncType>;

	auto methods = classMethods<FuncInfo::Class>();
	
	constexpr int index = MetaDetail::MethodInfoFinder<Func, 0,  decltype(methods)>::result;

	if constexpr (index < 0)
	{
		return std::false_type {};
	}
	else
	{
		return std::get<index>(methods);
	}
}



} // namespace Meta
} // namespace Runtime

//-----------------------------------------------------------------------------

#define CLASS_INFO(...)\
CLASS_ATTRIBUTES \
	__VA_ARGS__ \
END_CLASS_ATTRIBUTES


#define CLASS_BASE(...) ::Runtime::Meta::ClassBase<__VA_ARGS__>{}

#define DECLARE_CLASS_BASE(...) \
CLASS_ATTRIBUTES\
	CLASS_BASE(__VA_ARGS__)\
END_CLASS_ATTRIBUTES


//#define CLASS_NAMED_FIELD(field, name, ...) Runtime::Meta::FieldInfo{ (instance ? &instance->## field : nullptr), #field, __VA_ARGS__}
#define CLASS_NAMED_FIELD(field, name, ...) Runtime::Meta::FieldInfo{ instance == nullptr ? nullptr : &instance-> field, name, __VA_ARGS__}

#define CLASS_FIELD(field,...) CLASS_NAMED_FIELD(field, #field, __VA_ARGS__)

//#define CLASS_FIELDS(...) Runtime::Meta::ClassFields{[]<typename T>(T* instance) { \
//	return std::tuple{__VA_ARGS__}; \
//}} \

#define CLASS_FIELDS(...) Runtime::Meta::ClassFields{[](auto instance) { \
	return std::tuple{__VA_ARGS__}; \
}} \



#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL

#define CLASS_METHOD(Class_, method, ... ) Runtime::Meta::createMethodInfo<&Class_::method>(#method __VA_ARGS__ )
#define CLASS_NAMED_METHOD(class_, method, name, ...) Runtime::Meta::createMethodInfo<&class_::method>(name, __VA_ARGS__)

#else

#define CLASS_METHOD(Class_, method, ... ) Runtime::Meta::createMethodInfo<&Class_::method>(#method __VA_OPT__(,) __VA_ARGS__ )
#define CLASS_NAMED_METHOD(class_, method, name, ...) Runtime::Meta::createMethodInfo<&class_::method>(name, __VA_OPT__(,) __VA_ARGS__ )


#endif


#define CLASS_METHODS(...) Runtime::Meta::ClassMethods { __VA_ARGS__ }
