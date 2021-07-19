#pragma once
#include <string>
#include <type_traits>

namespace Runtime {

template<typename T>
inline constexpr bool IsStringRepresentable = std::is_convertible_v<T, std::string>;


template<typename T>
inline constexpr bool IsWStringRepresentable = std::is_convertible_v<T, std::wstring>;


template<typename T, typename C>
inline constexpr bool  StringRepresentableT =
	(std::is_same_v<C, char> && IsStringRepresentable<T>) ||
	(std::is_same_v<C, wchar_t> && IsWStringRepresentable<T>)
	;
}

