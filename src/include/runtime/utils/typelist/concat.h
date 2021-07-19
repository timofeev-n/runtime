#pragma once
#include "typelist.h"

namespace Runtime {
namespace TypeListDetail {

/// <summary>
///
/// </summary>
template<typename ...T>
struct Concat2TypeLists;


template<typename TL>
struct Concat2TypeLists<TL>
{
	static_assert(typelist::IsTypeList<TL>);
	using type = TL;
};


template<typename ...T1, typename ...T2>
struct Concat2TypeLists<TypeList<T1...>, TypeList<T2...>>
{
	using type = TypeList<T1..., T2...>;
};

/// <summary>
///
/// </summary>
template<typename ...>
struct Concat;


template<>
struct Concat<>
{
	using type = TypeList<> ;
};

template<typename TL, typename ... Tail>
struct Concat<TL, Tail...>
{
	using type = typename Runtime::TypeListDetail::Concat2TypeLists<TL, typename Concat<Tail...>::type>::type;
};


}

namespace typelist {


template<typename ... TL>
using Concat = typename Runtime::TypeListDetail::Concat<TL...>::type;


} // namespace typelist
} // namespace Runtime
