#pragma once

#include <runtime/utils/typelist.h>



namespace Runtime::TypeListDetail {

/// <summary>
///
/// </summary>
template<typename TL, template <typename, auto...> class Mapper>
struct Transform;


//template<template <typename, auto...> class Mapper, typename ... T>
//struct Transform<TypeList<T...>, Mapper>
//{
//	using type = TypeList<Mapper<T> ... >;
//};


template<template <typename, auto...> class Mapper, typename ... T>
struct Transform<TypeList<T...>, Mapper>
{
	using type = TypeList<typename Mapper<T>::type ... >;
};


}

namespace Runtime::typelist {

template<typename TL, template <typename, auto...> class Mapper>
using Transform = typename Runtime::TypeListDetail::Transform<TL, Mapper>::type;


}

