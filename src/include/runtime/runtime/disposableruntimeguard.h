#pragma once

#include <runtime/com/comptr.h>
#include <runtime/utils/disposable.h>
#include <runtime/utils/preprocessor.h>

namespace Runtime {

class DisposableRuntimeGuard final: public Disposable
{
public:

	DisposableRuntimeGuard(IRefCounted& disposable);

	DisposableRuntimeGuard();

	DisposableRuntimeGuard(const DisposableRuntimeGuard&) = delete;

	DisposableRuntimeGuard(DisposableRuntimeGuard&&) noexcept;

	~DisposableRuntimeGuard();

	DisposableRuntimeGuard& operator = (const DisposableRuntimeGuard&) = delete;

	DisposableRuntimeGuard& operator = (DisposableRuntimeGuard&&) noexcept;

	void dispose() override;

private:

	void* m_handle = nullptr;
};

}


#define rtdisposable(disposable) const Runtime::DisposableRuntimeGuard ANONYMOUS_VARIABLE_NAME(rtDisposableGuard__) {(disposable)}
