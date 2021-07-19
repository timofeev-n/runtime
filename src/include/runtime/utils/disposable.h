#pragma once
#include <runtime/com/interface.h>
#include <runtime/utils/scopeguard.h>

#include <type_traits>

namespace Runtime {


struct ABSTRACT_TYPE Disposable
{
	virtual void dispose() = 0;
};


template<typename T,
	std::enable_if_t<std::is_assignable_v<Disposable&, T&>, int> = 0>
inline void dispose(T& disposable) // requires(std::is_assignable_v<Disposable&, T&>)
{
	static_cast<Disposable&>(disposable).dispose();
}



template<typename>
class DisposableGuard;

}
