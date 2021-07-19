#pragma once
#include "typelist.h"

namespace Runtime::TypeListDetail {

template<typename, typename ...>
struct Append ;

template<typename, typename ... >
struct AppendHead;

template<typename ... T, typename ... El>
struct Append <TypeList<T ...>, El ...>
{
	using type = TypeList<T ..., El ...>;
};

template<typename ... T, typename ... El>
struct AppendHead <TypeList<T ...>, El ... >
{
	using type = TypeList<El ..., T ...>;
};

}

namespace Runtime::typelist {

template<typename TL, typename T, typename ... U>
using Append = typename Runtime::TypeListDetail::Append<TL, T, U ...>::type;

template<typename TL, typename T, typename ... U>
using AppendHead = typename Runtime::TypeListDetail::AppendHead<TL, T, U ... >::type;

}
