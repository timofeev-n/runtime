#include "pch.h"
#include "runtimescopeguard.h"
#include <runtime/runtime/runtime.h>
#include <runtime/threading/event.h>
#include <runtime/threading/setthreadname.h>
#include <runtime/utils/scopeguard.h>
#include <runtime/memory/rtstack.h>


using namespace Runtime;
using namespace Runtime::RuntimeLiterals;

RuntimeScopeGuard::RuntimeScopeGuard()
{
	Threading::Event signal;

	SCOPE_Leave {
		signal.wait();
	};

	thread = std::thread([&signal]
	{
		Threading::SetCurrentThreadName("Runtime Thread");
		auto runtime = RuntimeCore::create();
		Async::Scheduler::setDefault(RuntimeCore::instance().poolScheduler());

		SCOPE_Leave {
			Async::Scheduler::setDefault({});
		};

		signal.set();

		rtstack(1_Mb);

		runtime->run();
	});
}

RuntimeScopeGuard::~RuntimeScopeGuard()
{
	reset();
}

void RuntimeScopeGuard::reset()
{
	if (RuntimeCore::exists()) {
		RuntimeCore::instance().stop();
	}

	if (thread.joinable()) {
		thread.join();
	}

}
