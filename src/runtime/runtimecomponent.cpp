#include "pch.h"
#include "internalcomponents.h"
#include "componentids.h"



namespace Runtime {

namespace {

struct ComponentRegistration
{
	const std::string id;
	RuntimeComponent::Factory factory;
	void* const data;
	

	ComponentRegistration(std::string_view id_, RuntimeComponent::Factory factory_, void* data_ = nullptr)
		: id(id_)
		, factory(factory_)
		, data(data_)
	{}
};

std::list<ComponentRegistration>& componentsRegistry()
{
	static std::list<ComponentRegistration> registry = {
		{ ComponentIds::DefaultScheduler, RuntimeDefaultSchedulerInternalComponent() },
		{ ComponentIds::PoolScheduler, PoolSchedulerInternalComponent() },
		{ ComponentIds::TimerManager, TimerManagerInternalComponent() }
	};


	return (registry);
}

}


RuntimeComponentRegistrationHandle::RuntimeComponentRegistrationHandle(void* handle_): handle(handle_)
{}

RuntimeComponentRegistrationHandle::~RuntimeComponentRegistrationHandle()
{
	if (handle)
	{
		componentsRegistry().remove_if([entryPtr = reinterpret_cast<ComponentRegistration*>(this->handle)](const ComponentRegistration& entry) { return &entry == entryPtr; });
	}
}

RuntimeComponentRegistrationHandle::RuntimeComponentRegistrationHandle(RuntimeComponentRegistrationHandle&& other): handle(other.handle)
{
	other.handle = nullptr;
}

RuntimeComponentRegistrationHandle& RuntimeComponentRegistrationHandle::operator = (RuntimeComponentRegistrationHandle&& other)
{
	Assert2(!this->handle, "Operation not allowed");

	std::swap(this->handle, other.handle);
	return *this;
}

//-----------------------------------------------------------------------------
RuntimeComponentRegistrationHandle RuntimeComponent::registerComponent(std::string_view id, RuntimeComponent::Factory factory, void* data)
{
	auto& entry = componentsRegistry().emplace_back(id, factory, data);
	return RuntimeComponentRegistrationHandle(reinterpret_cast<void*>(&entry));
}

std::vector<ComponentEntry> createRuntimeComponents(uv_loop_t* uv)
{
	std::vector<ComponentEntry> components;
	components.reserve(componentsRegistry().size());

	for (ComponentRegistration& reg : componentsRegistry())
	{
		auto component = reg.factory(uv, reg.id, reg.data);
		Assert(component);
		components.emplace_back(ComponentEntry{reg.id, std::move(component)});
	}

	return components;
}

}
