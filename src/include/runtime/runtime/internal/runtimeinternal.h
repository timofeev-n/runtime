#pragma once
#include <runtime/runtime/runtime.h>
#include <EngineAssert.h>
#include <uv.h>



namespace Runtime {

struct RuntimeInternal
{
	static Async::Scheduler::Ptr scheduler();

	static uv_loop_t* uv();
};

} // namespace Runtime

#define SWITCH_RUNTIME_SCHEDULER \
	if (!::Runtime::RuntimeCore::isRuntimeThread())\
	{\
		::Runtime::Async::Scheduler::Ptr scheduler__ = Runtime::RuntimeInternal::scheduler(); \
		Assert(scheduler__);\
		co_await scheduler__;\
		Assert(::Runtime::RuntimeCore::isRuntimeThread()); \
	}\
