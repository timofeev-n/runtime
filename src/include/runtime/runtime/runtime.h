#pragma once

#include <runtime/com/comptr.h>
#include <runtime/async/scheduler.h>

#include <memory>


namespace Runtime {


/**
* 
*/
struct ABSTRACT_TYPE RuntimeCore
{
	enum class RunMode
	{
		Default,
		NoWait
	};

	static std::unique_ptr<RuntimeCore> create();

	static bool exists();

	static RuntimeCore& instance();

	static bool isRuntimeThread();

	virtual ~RuntimeCore() = default;

	/**
	* 
	*/
	virtual void run(RunMode = RunMode::Default) = 0;

	virtual void stop() = 0;
	// virtual Async::Scheduler::Ptr scheduler() = 0;
	virtual Async::Scheduler::Ptr poolScheduler() const = 0;

	virtual ComPtr<> findComponent(std::string_view id) const = 0;
};

}
