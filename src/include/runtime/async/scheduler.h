//◦ Playrix ◦
#pragma once

#include <runtime/com/comptr.h>
#include <runtime/com/weakcomptr.h>
#include <runtime/utils/uid.h>
#include <runtime/utils/functor.h>
#include <runtime/utils/preprocessor.h>
#include <runtime/compiler/coroutine.h>

#include <EngineAssert.h>

#include <thread>
#include <variant>


#define DEFAULT_CALLBACK __cdecl


namespace Runtime {

namespace AsyncDetail {

class SchedulerGuardImpl;

} // AsyncDetail


namespace Async {

class Scheduler;


/// <summary>
///
/// </summary>
class Scheduler : public virtual IRefCounted
{
public:

	using Ptr = ComPtr<Scheduler>;
	using WeakPtr = WeakComPtr<Scheduler>;

	struct InvocationGuard
	{
		InvocationGuard(Scheduler&);

		~InvocationGuard();

		const uint64_t threadId;
		Scheduler& scheduler;
		InvocationGuard* prev = nullptr;
	};

	using Callback = void (DEFAULT_CALLBACK *)(void* data1, void* data2) noexcept;

	struct CallbackEntry
	{
		Callback callback = nullptr;
		void* data1 = nullptr;
		void* data2 = nullptr;

		CallbackEntry() = default;

		CallbackEntry(Callback callback_, void* data1_, void* data2_ = nullptr): callback(callback_), data1(data1_), data2(data2_)
		{}

		explicit operator bool () const
		{
			return callback != nullptr;
		}

		void operator ()() const noexcept
		{
			Assert(callback);
			callback(data1, data2);
		}
	};


	using Invocation = std::variant<std::coroutine_handle<>, CallbackEntry> ;

	/**
	*/
	static Scheduler::Ptr getDefault();

	/**
	*/
	static Scheduler::Ptr getCurrent();

	/**
	*/
	static Scheduler::Ptr getInvoked();

	/**
	*/
	static Scheduler::Ptr currentThreadScheduler();

	/**
	*/
	static void setDefault(Scheduler::Ptr);

	/**
	*/
	static void setCurrentThreadScheduler(Scheduler::Ptr scheduler);

	

	static void bindSchedulerName(Scheduler::Ptr scheduler, std::string_view name);

	/**
	*/
	static Scheduler::Ptr findByName(std::string_view name);


	static void finalize(Scheduler::Ptr&& scheduler);

	/**
	*/
	void schedule(Invocation invocation) noexcept;

	void schedule(std::coroutine_handle<>) noexcept;

	void schedule(Callback, void* data1, void* data2 = nullptr) noexcept;


protected:

	static void invoke(Scheduler&, Invocation) noexcept;

	virtual ~Scheduler() noexcept = default;

	virtual void scheduleInvocation(Invocation) noexcept = 0;

	virtual void waitAnyActivity() noexcept = 0;

private:

	friend class AsyncDetail::SchedulerGuardImpl;
};



/*
* 
*/
struct SchedulerAwaiter
{
	Scheduler::Ptr scheduler;

	SchedulerAwaiter(Scheduler::Ptr scheduler_): scheduler(std::move(scheduler_))
	{
		Assert2(scheduler, "Schduler must be specified");
	}

	bool await_ready() const noexcept
	{
		return false;
	}

	void await_suspend(std::coroutine_handle<> continuation)
	{
		scheduler->schedule(std::move(continuation));
	}

	void await_resume() const noexcept
	{}
};


inline SchedulerAwaiter operator co_await(Scheduler::Ptr scheduler)
{
	return {std::move(scheduler)};
}

inline SchedulerAwaiter operator co_await(Scheduler::WeakPtr schedulerRef)
{
	auto scheduler = schedulerRef.acquire();
	Verify2(scheduler, "Scheduler instance expired");

	return {std::move(scheduler)};
}


} // naemspace async
} // namespace Runtime


namespace std {


}

//#ifdef _MSC_VER
//#pragma warning (pop)
//#endif

#define ASYNC_SWITCH_SCHEDULER(schedulerExpression_)\
	\
	{\
		Runtime::Async::Scheduler::Ptr scheduler__ = std::move(schedulerExpression_);\
		DEBUG_CHECK(scheduler__)\
		\
		if (Runtime::Async::Scheduler::getCurrent().get() != scheduler__.get())\
		{\
			co_await scheduler__;\
		}\
	}\


//
//#define ASYNC_SWITCH_SCHEDULER(uid)\
//{\
//	auto schedulerVar__ = Scheduler::find(uid);\
//	\
//	DEBUG_CHECK(schedulerVar__, "Specified scheduler:(%1) not found", uid)\
//	\
//	co_await schedulerVar__;\
//}\
//
//
//#define ASYNC_SWITCH_SCHEDULER_(scheduler)\
//{\
//	auto schedulerVar__ = std::move(scheduler);\
//	\
//	DEBUG_CHECK(schedulerVar__, "Specified is null")\
//	\
//	co_await schedulerVar__;\
//}\
//
//
//#define ASYNC_YIELD ASYNC_SWITCH_SCHEDULER_(Runtime::Async::Scheduler::getCurrent())

