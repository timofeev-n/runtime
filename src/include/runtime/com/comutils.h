//◦ Playrix ◦
#pragma once
#include <runtime/com/ianything.h>
#include <runtime/meta/classinfo.h>


namespace Runtime::Detail {


template<typename Target, typename T, typename ... Base>
inline bool castHelper(T& instance, Target*& target, TypeList<Base...>) {

	if constexpr (std::is_convertible_v<T*, Target*>) {
		target = &static_cast<Target&>(instance);
		return true;
	}
	else {
		return (castHelper(static_cast<Base&>(instance), target, Meta::ClassDirectBase<Base>{}) || ... );
	}
}


template<typename T, typename ... Base>
inline bool rtCastHelper(T& instance, const std::type_info& targetTypeId, void** target, TypeList<Base...>) {

	const std::type_info& instanceTypeId = typeid(T);
	if (instanceTypeId == targetTypeId) {
		*target = reinterpret_cast<void*>(&instance);
		return true;
	}

	return (rtCastHelper(static_cast<Base&>(instance), targetTypeId, target, Meta::ClassDirectBase<Base>{}) || ... );
}


template<typename T, typename ... Base>
constexpr bool IsConvertibleHelper(TypeList<Base...>) {
	return (std::is_convertible_v<Base*, T*> || ...);
}


template<typename ... Base>
constexpr bool RtIsHelper(const std::type_info& targetTypeId, TypeList<Base...>) {
	return ((targetTypeId == typeid(Base)) || ...);
};

}


namespace Runtime::Com {


template<typename Target, typename T>
inline Target* cast(T& instance)
{
	using namespace Runtime::Detail;

	using Type = std::remove_reference_t<std::remove_const_t<T>>;

	Target* target = nullptr;
	return castHelper(const_cast<Type&>(instance), target, Meta::ClassDirectBase<Type>{}) ? target : nullptr;
}


template<typename T>
void* rtCast(T& instance, const std::type_info& targetType)
{
	using namespace Runtime::Detail;

	using Type = std::remove_reference_t<std::remove_const_t<T>>;

	if (targetType == typeid(IAnything)) {
		IAnything* anything = Com::cast<IAnything>(instance);

		return reinterpret_cast<void*>(anything);
	}
	else if (targetType == typeid(IRefCounted)) {
		IRefCounted* refCounted = Com::cast<IRefCounted>(instance);

		return reinterpret_cast<void*>(refCounted);
	}

	void* target = nullptr;
	return rtCastHelper(const_cast<Type&>(instance), targetType, &target, Meta::ClassDirectBase<Type>{}) ? target : nullptr;
}



template<typename T>
bool rtIs(const std::type_info& targetTypeId)
{
	using Type = std::remove_reference_t<std::remove_const_t<T>>;
	using Bases = Meta::ClassAllUniqueBase<Type>;

	if (targetTypeId == typeid(IAnything))  {
		return Detail::IsConvertibleHelper<IAnything*>(Bases{});
	}
	else if (targetTypeId == typeid(IRefCounted)) {
		return Detail::IsConvertibleHelper<IRefCounted*>(Bases{});
	}

	return Detail::RtIsHelper(targetTypeId, Bases{});
}

}
