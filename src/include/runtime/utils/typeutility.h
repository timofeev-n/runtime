#pragma once
#include <type_traits>

namespace Runtime {

namespace Detail {

/// <summary>
///
/// </summary>
template<template <typename ... X> class TemplatedClass, typename T>
struct IsTemplateOfHelper
{
	static std::false_type helper(...);

	template<typename ... U>
	static std::true_type helper(const TemplatedClass<U ...>&);

	constexpr static bool value = decltype(helper(std::declval<T>()))::value;
};


/// <summary>
///
/// </summary>
template<template <typename ...> class TemplatedClass>
struct IsTemplateOfHelper<TemplatedClass, void>
{
	constexpr static bool value = false;
};

} // namespace Detail




/// <summary>
/// Tell that type is an instantiation of base template class.
/// </summary>
template<template <typename ...> class TemplateClass, typename T>
inline constexpr bool IsTemplateOf = Detail::IsTemplateOfHelper<TemplateClass, T>::value;

template<typename T>
std::add_lvalue_reference_t<T> LValueRef();


template<typename T>
std::add_const_t<std::add_lvalue_reference_t<T>> ConstLValueRef();



template<typename T, typename ... U>
inline constexpr bool AnyOf = (std::is_same_v<T, U> || ...);

}
