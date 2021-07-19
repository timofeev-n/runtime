#pragma once


#if defined(_MSC_VER)

#if __cpp_impl_coroutine
#include <coroutine>
#else

#include <experimental/coroutine>
#define STD_CORO std::experimental

namespace std {

template <typename Promise = void>
using coroutine_handle = ::std::experimental::coroutine_handle<Promise>;

using suspend_never = ::std::experimental::suspend_never;
using suspend_always = ::std::experimental::suspend_always;

}


#endif

#elif __has_include (<experimental/coroutine>)

#include <experimental/coroutine>

namespace std {

#define STD_CORO std::experimental

template <typename Promise = void>
using coroutine_handle = ::std::experimental::coroutine_handle<Promise>;

using suspend_never = ::std::experimental::suspend_never;

using suspend_always = ::std::experimental::suspend_always;

}

#else

#error Do not known how to using coroutine with that compiler

#endif

