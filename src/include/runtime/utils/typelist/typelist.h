#pragma once
#include <type_traits>
#include <tuple>

namespace Runtime {


/**
* 
*/
template<typename ...T>
struct TypeList
{
	static constexpr size_t Size = sizeof ... (T);
};


namespace TypeListDetail {

template<typename>
struct TypeListOf;

template<typename...T>
struct TypeListOf<std::tuple<T...>> 
{
	using type = TypeList<T...>;
};

/// <summary>
///
/// </summary>
template<typename TL, typename T, int Index>
struct ElementIndex;


template<typename T, int Index>
struct ElementIndex<TypeList<>, T, Index>
{
	inline constexpr static int value = -1;
};


template<typename Head, typename ... Tail, typename T, int Index>
struct ElementIndex<TypeList<Head, Tail...>, T, Index>
{
	inline constexpr static int value = std::is_same_v<T, Head> ?
		Index :
		ElementIndex<TypeList<Tail...>, T, Index + 1>::value;
};


template<typename T>
struct IsTypeList : std::false_type
{};

template<typename ...U>
struct IsTypeList<TypeList<U...>> : std::true_type
{};

} // namespace TypeListDetail


namespace typelist {

template<typename T>
using TypeListOf = typename Runtime::TypeListDetail::TypeListOf<T>::type;

template<typename TL, typename T>
inline constexpr int ElementIndex = Runtime::TypeListDetail::ElementIndex<TL, T, 0>::value;

template<typename T>
inline constexpr bool IsTypeList = Runtime::TypeListDetail::IsTypeList<T>::value;


}} // namespace Runtime::typelist
