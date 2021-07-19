#pragma once
#include "typelist.h"

namespace Runtime::typelist {
namespace TypeListDetail {

template<typename TL, typename El>
struct Contains;


template<typename ... T, typename El>
struct Contains<TypeList<T...>, El>
{
	constexpr static bool value = (std::is_same_v<T, El> || ...);
};

template<typename TL, typename SubList>
struct ContainsAll;

template<typename TL, typename ... T>
struct ContainsAll<TL, TypeList<T...>>
{
	constexpr static bool value = (Contains<TL, T>::value && ...);
};

} // namespace TypeListDetail

template<typename TL, typename T>
inline constexpr bool Contains = TypeListDetail::Contains<TL, T>::value;

template<typename TL, typename SubList>
inline constexpr bool ContainsAll = TypeListDetail::ContainsAll<TL, SubList>::value;

}
