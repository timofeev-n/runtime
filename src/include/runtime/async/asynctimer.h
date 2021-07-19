#pragma once
#include <runtime/async/taskbasetypes.h>
#include <runtime/com/ianything.h>
#include <runtime/memory/allocator.h>

#include <chrono>
#include <optional>

namespace Runtime::Async {

/**
*/
struct ABSTRACT_TYPE AsyncTimer : virtual IRefCounted
{
	using Ptr = ComPtr<AsyncTimer>;

	virtual ~AsyncTimer() = default;
	virtual Task<bool> tick() = 0;
	virtual void stop();
};

/**
*/
AsyncTimer::Ptr createRuntimeTimer(std::chrono::milliseconds dueTime, std::optional<std::chrono::milliseconds> repeat = std::nullopt, ComPtr<Allocator> = {});

}

namespace Runtime::AsyncDetail {

/**
*/
void runtimeResumeAfter(std::chrono::milliseconds, Async::Scheduler::Ptr, std::coroutine_handle<>) noexcept;

}
