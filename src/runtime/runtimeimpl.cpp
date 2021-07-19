#include "pch.h"
#include "runtime/runtime/runtime.h"
#include "runtime/runtime/internal/runtimeinternal.h"
#include "runtime/runtime/internal/runtimeallocator.h"
#include "runtime/runtime/internal/uvutils.h"
#include "runtime/runtime/internal/runtimecomponent.h"
#include "runtime/utils/disposable.h"
#include "runtime/memory/rtstack.h"
#include "runtime/com/comclass.h"

#include "componentids.h"


namespace Runtime {

using namespace ::Runtime::Async;

//-----------------------------------------------------------------------------

namespace {

struct RuntimeState
{
	RuntimeCore* runtime = nullptr;
	Allocator::Ptr runtimePoolAllocator;
	AllocatorMemoryResource runtimePoolMemoryResource;
	
	RuntimeState();
};

enum class RuntimeStateAccess
{
	Initialize,
	Reset,
	Acquire
};


RuntimeState* getRuntimeState(RuntimeStateAccess what = RuntimeStateAccess::Acquire)
{
	static std::optional<RuntimeState> runtimeState;

	if (what == RuntimeStateAccess::Initialize)
	{
		Assert2(!runtimeState, "Runtime state already initialized");
		return &runtimeState.emplace();
	}
	else if (what == RuntimeStateAccess::Reset)
	{
		Assert2(runtimeState, "Runtime state expected to be initialized");
		Assert(runtimeState->runtimePoolAllocator->refsCount() == 1);

		runtimeState.reset();
		return nullptr;
	}

	return runtimeState ? &(*runtimeState) : nullptr;
}


inline RuntimeState& requireRuntimeState()
{
	auto runtimeState = getRuntimeState();
	Assert2(runtimeState, "Runtime does not created");

	return *runtimeState;
}


struct RuntimeStateKeeper
{
	RuntimeStateKeeper(RuntimeCore* runtime_, uv_loop_t* uv)
	{
		getRuntimeState(RuntimeStateAccess::Initialize)->runtime = runtime_;

		_putenv_s("UV_THREADPOOL_SIZE", "10");
		UV_RUNTIME_CHECK(uv_loop_init(uv))
		uv_loop_set_data(uv, runtime_);
	}

	~RuntimeStateKeeper()
	{
		getRuntimeState(RuntimeStateAccess::Reset);
	}
};


RuntimeState::RuntimeState(): runtimePoolMemoryResource([]() -> Allocator*
{
	return requireRuntimeState().runtimePoolAllocator.get();
})
{
	runtimePoolAllocator = createPoolAllocator({
		/*.concurrent = */true,
		/*.granularity = */PoolAllocatorConfig::DefaultGranularity
	});
}

} // namespace


//RuntimeInternal::RuntimeInternal(): m_runtimeThreadId(std::this_thread::get_id())
//{}


/**
*/
class RuntimeImpl : public RuntimeCore
{
	CLASS_INFO(
		CLASS_BASE(RuntimeCore)
	)

	SINGLETON_MEMOP(RuntimeImpl)

public:

	RuntimeImpl()
		: m_runtimeStateKeeper(this, &m_uv)
		, m_threadId(std::this_thread::get_id())
		, m_components(createRuntimeComponents(&m_uv))
	{
	}

	~RuntimeImpl()
	{
		const auto hasReferences = [this]() -> bool
		{
			return std::any_of(m_components.begin(), m_components.end(), [](const ComponentEntry& entry) {
				return entry.component->componentHasWorks() || entry.component->refsCount() > 1;
			});
		};

		for (const auto& entry : m_components)
		{
			if (Disposable* const disposable = entry.component->as<Disposable*>())
			{
				disposable->dispose();
			}
		}

		while (hasReferences())
		{
			uv_run(&m_uv, UV_RUN_NOWAIT);
		}

		m_components.clear();

		while (uv_loop_alive(&m_uv))
		{
			if (uv_run(&m_uv, UV_RUN_NOWAIT) == 0)
			{
				break;
			}
		}

		UV_RUNTIME_CHECK(uv_loop_close(&m_uv))
	}

	void run(RunMode mode) override
	{
		uv_run(&m_uv, UV_RUN_DEFAULT);
	}

	void stop() override
	{
		scheduler()->schedule([](void* ptr, void*) noexcept
		{
			RuntimeImpl& this_ = *reinterpret_cast<RuntimeImpl*>(ptr);
			uv_stop(&this_.m_uv);
		}, this);
	}

	Async::Scheduler::Ptr scheduler()
	{
		ComPtr<> component = findComponent(ComponentIds::DefaultScheduler);
		Assert(component);

		return component;
	}

	Async::Scheduler::Ptr poolScheduler() const override
	{
		ComPtr<> component = findComponent(ComponentIds::PoolScheduler);
		Assert(component);

		return component;
	}

	ComPtr<> findComponent(std::string_view id) const
	{
		if (const auto entry = std::find_if(m_components.begin(), m_components.end(), [id](const ComponentEntry& c){ return c.id == id; }); entry != m_components.end())
		{
			return entry->component;
		}

		return ComPtr{};
	}

private:

	uv_loop_t* uv()
	{
		return &m_uv;
	}

	const RuntimeStateKeeper m_runtimeStateKeeper;
	const std::thread::id m_threadId;

	uv_loop_t m_uv;
	std::vector<ComponentEntry> m_components;
	
	friend struct RuntimeInternal;
	friend struct RuntimeCore;
};


//-----------------------------------------------------------------------------

std::unique_ptr<RuntimeCore> RuntimeCore::create()
{
	return std::make_unique<RuntimeImpl>();
}


bool RuntimeCore::exists()
{
	auto* const rtState = getRuntimeState();
	return rtState && rtState->runtime ;
}


RuntimeCore& RuntimeCore::instance()
{
	RuntimeState* const rtState = getRuntimeState();
	Assert2(rtState && rtState->runtime, "Runtime instance not exists");
	return *rtState->runtime;
}


bool RuntimeCore::isRuntimeThread()
{
	return static_cast<RuntimeImpl&>(RuntimeCore::instance()).m_threadId == std::this_thread::get_id();
}


Scheduler::Ptr RuntimeInternal::scheduler()
{
	return static_cast<RuntimeImpl&>(RuntimeCore::instance()).scheduler();
}


uv_loop_t* RuntimeInternal::uv()
{
	Assert(RuntimeCore::isRuntimeThread());
	return &static_cast<RuntimeImpl&>(RuntimeCore::instance()).m_uv;
}



void* runtimePoolAllocate(size_t size) noexcept
{
	return requireRuntimeState().runtimePoolAllocator->alloc(size);
}


void runtimePoolFree(void* ptr, std::optional<size_t> size) noexcept
{
	return requireRuntimeState().runtimePoolAllocator->free(ptr, size);
}

Allocator::Ptr& runtimePoolAllocator()
{
	return requireRuntimeState().runtimePoolAllocator;
}

std::pmr::memory_resource* runtimePoolMemoryResource()
{
	return &requireRuntimeState().runtimePoolMemoryResource;
}

//-----------------------------------------------------------------------------

//void* RuntimePoolMemoryResource::do_allocate(size_t size, size_t)
//{
//	return requireRuntimeState().runtimePoolAllocator->alloc(size);
//}
//
//void RuntimePoolMemoryResource::do_deallocate(void* ptr, size_t size, size_t)
//{
//	return requireRuntimeState().runtimePoolAllocator->free(ptr, size);
//}
//
//bool RuntimePoolMemoryResource::do_is_equal(const std::pmr::memory_resource& other) const noexcept
//{
//	return static_cast<const std::pmr::memory_resource*>(this) == &other;
//}

}


