#pragma once
#include <runtime/serialization/valuerepresentation/stringrepresentable.h>
#include <runtime/utils/typeutility.h>
#include <string>
#include <string_view>

namespace Runtime {
namespace Detail { 

template<typename T>
decltype(ToString(ConstLValueRef<T>())) ToStringResultHelper(int);

template<typename>
std::false_type ToStringResultHelper(...);

//template<typename T>
//decltype(Parse(std::string_view{})) ParseResultHelper();

//template<typename>
//std::false_type ParseResultHelper(...);

} // namespace Detail

template<typename T>
inline constexpr bool ToStringIsApplicable = IsStringRepresentable<decltype(Detail::ToStringResultHelper<T>(int{}))>;

//template<typename T>
//inline constexpr bool ParseIsApplicable = std::is_assignable_v<T&, decltype(Detail::ParseResultHelper<T>(std::string_view{}))>;


inline std::string ToString(std::string_view str) {
	return std::string{str};
}

//inline std::string Parse(std::string_view str) {
//	return std::string{str};
//}

//template<typename T>
//concept StringRepresentable = requires(const T &value) {
//	{ toString(value) } -> internal__::Convertible<std::string>;
//};
//
//
//template<typename T>
//concept WStringRepresentable = requires(const T &value) {
//	{ toWString(value) } -> internal__::Convertible<std::wstring>;
//};
//
//
//
//template<typename T, typename C>
//concept StringRepresentableT =
//	(std::is_same_v<C, char> && StringRepresentable<T>) ||
//	(std::is_same_v<C, wchar_t> && WStringRepresentable<T>)
//	;
//
//
//
//template<typename C, typename T>
//inline std::basic_string<C> toStringT(const T& value) requires StringRepresentableT<T,C> {
//	if constexpr (std::is_same_v<C, char>) {
//		return toString(value);
//	}
//	else {
//		return toWString(value);
//	}
//}

} // namespace Runtime
