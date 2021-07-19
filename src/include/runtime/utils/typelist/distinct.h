#pragma once
#include "typelist.h"

namespace Runtime::TypeListDetail {

/// <summary>
///
/// </summary>
template<typename TL, typename Result = TypeList<>>
struct Distinct;


template<typename Result>
struct Distinct<TypeList<>, Result> {
	using type = Result;
};


template<typename Element, typename ... Tail, typename ... Result>
struct Distinct<TypeList<Element, Tail...>, TypeList<Result...>>
{
	static constexpr bool NotExists = typelist::ElementIndex<TypeList<Result...>, Element> < 0;

	using type = std::conditional_t<NotExists,
		typename Distinct<TypeList<Tail...>, TypeList<Result..., Element>>::type,
		typename Distinct<TypeList<Tail...>, TypeList<Result...>>::type
	>;
};

} //namespace Runtime::TypeListDetail

namespace Runtime::typelist {

template<typename TL>
using Distinct = typename Runtime::TypeListDetail::Distinct<TL>::type;

}
