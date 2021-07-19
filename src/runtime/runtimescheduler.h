#pragma once
#include "runtime/async/scheduler.h"
#include "runtime/com/interface.h"
#include "runtime/meta/classinfo.h"
#include <uv.h>

namespace Runtime {

/**
*/
class ABSTRACT_TYPE RuntimeScheduler : public Async::Scheduler
{
	CLASS_INFO(
		CLASS_BASE(Async::Scheduler)
	)

	virtual bool hasPendingTasks() = 0;

protected:
	virtual uv_loop_t* uvLoop() const = 0;
	

private:

	virtual void waitAnyActivity() noexcept final;
};

}
