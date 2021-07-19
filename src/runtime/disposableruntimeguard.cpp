#include "pch.h"
#include "runtime/runtime/runtime.h"
#include "runtime/runtime/disposableruntimeguard.h"
#include "runtime/runtime/internal/runtimecomponent.h"
#include "runtime/runtime/internal/runtimeallocator.h"
#include "runtime/com/comclass.h"
#include "runtime/memory/rtstack.h"
#include "runtime/threading/critical_section.h"

namespace Runtime {

class DisposableRuntimeGuardComponent final : public RuntimeComponent, public Disposable
{
	COMCLASS_(RuntimeComponent, Disposable)

public:

	static constexpr std::string_view ComponentId = "disposables";

	static DisposableRuntimeGuardComponent& getThis()
	{
		ComPtr<> component = RuntimeCore::instance().findComponent(ComponentId);
		Assert(component);

		return component->as<DisposableRuntimeGuardComponent&>();
	}

	void registerInstance(WeakComPtr<> ptr)
	{
		lock_(m_mutex);
		Assert(!m_isDisposed);
		Assert(ptr);

		m_instances.emplace_back(std::move(ptr));
	}

	void unregisterInstance(IWeakReference* targetPtr)
	{
		if (!targetPtr)
		{
			return;
		}

		lock_(m_mutex);

		auto iter = std::find_if(m_instances.begin(), m_instances.end(), [targetPtr](const WeakComPtr<>& ptr) { return ptr.get() == targetPtr; });
		if (iter != m_instances.end())
		{
			m_instances.erase(iter);
		}
	}


private:

	bool componentHasWorks() override
	{
		lock_(m_mutex);
		return !m_instances.empty();
	}


	void dispose() override
	{
		rtstack();

		std::vector<ComPtr<>, RtStackStdAllocator<ComPtr<>>> instances;

		{
			lock_(m_mutex);
			if (const bool alreadyDisposed = m_isDisposed.exchange(true); alreadyDisposed)
			{
				return;
			}

			instances.reserve(m_instances.size());

			for (auto iter = m_instances.begin(); iter != m_instances.end();)
			{
				if (auto instance = iter->acquire(); instance)
				{
					instances.emplace_back(std::move(instance));
					++iter;
				}
				else
				{
					iter = m_instances.erase(iter);
				}
			}
		}

		for (ComPtr<>& instance : instances)
		{
			if (Disposable* const disposable = instance->as<Disposable*>(); disposable)
			{
				try
				{
					disposable->dispose();
				}
				catch (const std::exception& exc)
				{
					LOG_ERROR("Unexpected exception while disposing:", exc.what());
				}
			}
		}
	}

	std::list<WeakComPtr<>/*, RuntimeStdAllocator<WeakComPtr<>>*/> m_instances;
	Threading::CriticalSection m_mutex;
	std::atomic_bool m_isDisposed = false;
};


//-----------------------------------------------------------------------------
DisposableRuntimeGuard::DisposableRuntimeGuard() = default;

DisposableRuntimeGuard::DisposableRuntimeGuard(IRefCounted& disposable)
{
	if (disposable.is<Disposable>())
	{
		WeakComPtr<IRefCounted> weakPtr{Com::Acquire{&disposable}};
		m_handle = weakPtr.get();
		Assert(m_handle);
		DisposableRuntimeGuardComponent::getThis().registerInstance(std::move(weakPtr));
	}
}

DisposableRuntimeGuard::DisposableRuntimeGuard(DisposableRuntimeGuard&& other) noexcept : m_handle(other.m_handle)
{
	this->dispose();
}

DisposableRuntimeGuard::~DisposableRuntimeGuard()
{
	DisposableRuntimeGuardComponent::getThis().unregisterInstance(reinterpret_cast<Runtime::IWeakReference*>(m_handle));
}

DisposableRuntimeGuard& DisposableRuntimeGuard::operator = (DisposableRuntimeGuard&& other) noexcept
{
	this->dispose();
	Assert(!m_handle);
	std::swap(m_handle, other.m_handle);

	return *this;
}

void DisposableRuntimeGuard::dispose()
{
	IWeakReference* const ptr = reinterpret_cast<Runtime::IWeakReference*>(m_handle);
	m_handle = nullptr;
	DisposableRuntimeGuardComponent::getThis().unregisterInstance(ptr);
}


namespace {

[[maybe_unused]] const auto registration = RuntimeComponent::registerComponent(DisposableRuntimeGuardComponent::ComponentId, [](uv_loop_t*, std::string_view, void*) noexcept -> RuntimeComponent::Ptr
{
	return Com::createInstanceSingleton<DisposableRuntimeGuardComponent, RuntimeComponent>();
}
, nullptr);

} // namespace

} // namespace Runtime

