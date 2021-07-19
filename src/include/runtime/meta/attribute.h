#pragma once
#include <runtime/utils/typelist/contains.h>
#include <runtime/utils/typelist/transform.h>

#include <tuple>


namespace Runtime {
namespace Meta {

/**
*/
struct Attribute
{};


/**
*/
template<typename A>
struct AttributeValue
{
	using AttributeT = A;
};

} // namespace Meta

//-----------------------------------------------------------------------------
namespace MetaDetail {


/// <summary>
///
/// </summary>
struct ClassAttributesStorage
{};


/// <summary>
///
/// </summary>
//template<typename T>
//concept ClassAttributesProvider = requires
//{
//	requires std::is_base_of_v<ClassAttributesStorage, typename T::ClassAttributes__>;
//	{ T::ClassAttributes__::attributes() } -> concepts::TemplateOf<std::tuple>;
//};
//
//


template<typename T,
	typename = std::enable_if_t<std::is_base_of_v<ClassAttributesStorage, typename T::ClassAttributesDefinition>>
>
constexpr std::true_type classAttributesDefinedHelper(int);


template<typename T>
constexpr std::false_type classAttributesDefinedHelper(...);


template<typename T>
inline constexpr bool ClassAttributesDefined = decltype(MetaDetail::classAttributesDefinedHelper<T>(int{}))::value;



template<typename T,
	std::enable_if_t<MetaDetail::ClassAttributesDefined<T>, int> = 0>
decltype(auto) classAttributes() {
	return typename T::ClassAttributesDefinition::attributes();
}

template<typename T,
	std::enable_if_t<!MetaDetail::ClassAttributesDefined<T>, int> = 0>
constexpr std::tuple<> classAttributes() {
	return {};
}


template<typename T, typename A>
struct ExternalAttribute
{
	inline constexpr static bool Defined = false;
};


template<typename T>
constexpr decltype(std::declval<typename T::AttributeT>(), bool{}) isAttributeValue(int) { return true; }

template<typename T>
constexpr inline bool isAttributeValue(...) { return false; }

#if 0

template<typename T>
requires requires {typename T::AttributeT;}
constexpr bool IsAttributeValue__() { return true; }

template<typename T>
constexpr bool IsAttributeValue__() { return false; }

#endif


template<typename A, bool = isAttributeValue<A>(0)>
struct UnwrapAttributeHelper
{
	using type = typename A::AttributeT;
};

template<typename A>
struct UnwrapAttributeHelper<A, false>
{
	using type = A;
};


// //#ifdef __clang__

 template<typename T>
 using UnwrapAttribute = UnwrapAttributeHelper<T>;

// //#else
// //#endif



template<typename T, typename A>
constexpr bool attributeDefined() {
	if constexpr (ExternalAttribute<T, A>::Defined) {
		return true;
	}
	else {
		using ClassSpecifiedAttributes = typelist::TypeListOf<decltype(classAttributes<T>())>;
		using AttributeTypes = typelist::Transform<ClassSpecifiedAttributes, UnwrapAttribute>;

		return typelist::Contains<AttributeTypes, A>;
	}
}

} // namespace MetaDetail

//-----------------------------------------------------------------------------
namespace Meta {

/// <summary>
///
/// </summary>
template<typename T, typename A>
inline constexpr bool AttributeDefined = Runtime::MetaDetail::attributeDefined<T,A>();

/// <summary>
///
/// </summary>
template<typename T, typename A>
auto attributeValue()
{
	using namespace Runtime::MetaDetail;

	if constexpr (ExternalAttribute<T, A>::Defined) {
		return ExternalAttribute<T, A>::value();
	}
	else {
		auto attributes = classAttributes<T>();

		using ClassSpecifiedAttributes = typelist::TypeListOf<decltype(attributes)>;
		using Attributes = typelist::Transform<ClassSpecifiedAttributes, UnwrapAttribute>;

		if constexpr (constexpr int AttributeIndex = typelist::ElementIndex<Attributes, A>; AttributeIndex >= 0) {
			return std::get<AttributeIndex>(attributes);
		}
		else {
			static_assert(AttributeIndex >= 0, "Requested attribute not defined for specified type");

			return 0;
		}
	}
}

/// <summary>
/// 
/// </summary>
template<typename T, typename A>
using AttributeValueType = decltype(attributeValue<T, A>());

} // namespace Meta
} // naemspace Runtime


#define CLASS_ATTRIBUTES \
public:\
\
struct ClassAttributesDefinition final : public ::Runtime::MetaDetail::ClassAttributesStorage \
{\
	static auto attributes() { \
		return std::tuple {


#define END_CLASS_ATTRIBUTES \
		}; \
	} \
\
};
