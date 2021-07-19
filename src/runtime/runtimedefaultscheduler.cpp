#include "pch.h"
#include "internalcomponents.h"
#include "runtime/runtime/internal/runtimeallocator.h"
#include "runtime/runtime/internal/uvutils.h"
#include "runtime/runtime/internal/runtimecomponent.h"
#include "runtime/async/scheduler.h"

#include "runtime/com/comclass.h"
#include "runtime/utils/scopeguard.h"
#include "runtime/threading/critical_section.h"




using namespace Runtime::Async;

namespace Runtime {

class RuntimeDefaultScheduler final : public Scheduler, public RuntimeComponent
{
	COMCLASS_(Scheduler, RuntimeComponent)

public:

	RuntimeDefaultScheduler(uv_loop_t* uv_)
	{
		uv_async_init(uv_, m_async, [](uv_async_t* handle) noexcept
		{
			RuntimeDefaultScheduler& this_ = *reinterpret_cast<RuntimeDefaultScheduler*>(handle->data);
			this_.processInvocations();
		});

		uv_handle_set_data(m_async, this);
	}

	~RuntimeDefaultScheduler()
	{}

	void scheduleInvocation(Invocation invocation) noexcept override
	{
		{
			std::lock_guard lock{m_mutex};
			m_invocations.emplace_back(std::move(invocation));
		}

		uv_async_send(m_async);
	}


private:

	void processInvocations() noexcept
	{
		Scheduler::InvocationGuard guard{*this};

		while (true)
		{
			std::pmr::list<Invocation> invocations(runtimePoolMemoryResource());

			{
				std::lock_guard lock{m_mutex};
				if (m_invocations.empty())
				{
					break;
				}

				m_workInProgress = true;
				invocations.splice(invocations.begin(), m_invocations);
			}

			SCOPE_Leave {
				std::lock_guard lock{m_mutex};
				m_workInProgress = false;
			};

			for (auto& invocation : invocations)
			{
				Scheduler::invoke(*this, std::move(invocation));
			}
		}
	}

	uv_loop_t* uvLoop() const
	{
		return m_async->loop;
	}

	void waitAnyActivity() noexcept override 
	{
		Halt("Unexpected ::waitAnyActivity invocation");
		//uv_loop_t* const uv = this->uvLoop();
		//while (this->hasWorks())
		//{
		//	uv_run(uv, UV_RUN_ONCE);
		//}
	}

	bool componentHasWorks() override
	{
		std::lock_guard lock{m_mutex};
		return !m_invocations.empty() || m_workInProgress;
	}

	UvHandle<uv_async_t> m_async;
	Threading::CriticalSection m_mutex;
	std::pmr::list<Scheduler::Invocation> m_invocations{runtimePoolMemoryResource()};
	bool m_workInProgress = false;
};


RuntimeComponent::Factory RuntimeDefaultSchedulerInternalComponent() {
	return [](uv_loop_t* uv, std::string_view, void*) noexcept -> RuntimeComponent::Ptr {
		return Com::createInstanceSingleton<RuntimeDefaultScheduler, RuntimeComponent>(uv);
	};
}


} // namespace Runtime

