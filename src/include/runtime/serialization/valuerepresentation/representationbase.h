#pragma once
#include <type_traits>

namespace Runtime {

template<typename T, typename V = T>
struct ValueRepresentationBase
{
	using Type = std::remove_const_t<T>;
	using ValueType = std::remove_const_t <V>;
};

}
