#include "pch.h"
#include "runtimescheduler.h"
#include "uv.h"

namespace Runtime {

void RuntimeScheduler::waitAnyActivity() noexcept {
	uv_loop_t* const uv = this->uvLoop();
	while (this->hasPendingTasks()) {
		uv_run(uv, UV_RUN_ONCE);
	}
}

}
