#pragma once

#include <runtime/com/ianything.h>
#include <runtime/com/comptr.h>
#include <runtime/utils/functor.h>
#include <runtime/utils/disposable.h>

#include <uv.h>


#ifdef RUNTIME_BUILD
#include <vector>
#endif


namespace Runtime {


struct [[nodiscard]] RuntimeComponentRegistrationHandle
{
	RuntimeComponentRegistrationHandle(void*);

	~RuntimeComponentRegistrationHandle();

	RuntimeComponentRegistrationHandle(RuntimeComponentRegistrationHandle&) = delete;

	RuntimeComponentRegistrationHandle(RuntimeComponentRegistrationHandle&&);

	RuntimeComponentRegistrationHandle& operator = (const RuntimeComponentRegistrationHandle&) = delete;

	RuntimeComponentRegistrationHandle& operator = (RuntimeComponentRegistrationHandle&&);

private:

	void* handle = nullptr;
};


struct ABSTRACT_TYPE RuntimeComponent : IRefCounted
{
	using Ptr = ComPtr<RuntimeComponent>;
	using Factory = RuntimeComponent::Ptr (*) (uv_loop_t*, std::string_view, void*) noexcept;
	static RuntimeComponentRegistrationHandle registerComponent(std::string_view id, RuntimeComponent::Factory factory, void* data);

	virtual bool componentHasWorks() = 0;
};


#ifdef RUNTIME_BUILD

struct ComponentEntry
{
	const std::string_view id;
	const RuntimeComponent::Ptr component;
};

std::vector<ComponentEntry> createRuntimeComponents(uv_loop_t* uv);
#endif

}
