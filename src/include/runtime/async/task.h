//◦ Playrix ◦
#pragma once

#include <runtime/async/taskbasetypes.h>
#include <runtime/async/core/coretask.h>
#include <runtime/async/core/coretasklinkedlist.h>
#include <runtime/async/scheduler.h>
#include <runtime/async/asynctimer.h>
#include <runtime/utils/typeutility.h>
#include <runtime/utils/scopeguard.h>
#include <runtime/compiler/coroutine.h>

#include <EngineAssert.h>

#include <chrono>
#include <initializer_list>
#include <optional>


namespace Runtime {
namespace AsyncDetail {

void runtimeResumeAfter(std::chrono::milliseconds, Async::Scheduler::Ptr, std::coroutine_handle<>) noexcept;



/**
	Rvalue reference task awaiter
*/
template<typename T>
struct RvTaskAwaiter
{
	Async::Task<T> task;

	RvTaskAwaiter(Async::Task<T>&& task_): task(std::move(task_))
	{}

	bool await_ready() const noexcept
	{
		return task.ready();
	}

	void await_suspend(std::coroutine_handle<> coro) noexcept
	{
		using namespace Runtime::Async;

		CoreTask& coreTask = static_cast<CoreTaskPtr&>(this->task).coreTask();
		coreTask.setContinuation({Scheduler::Invocation{std::move(coro)}, Scheduler::getCurrent()});
	}

	T await_resume()
	{
		if constexpr (std::is_same_v<void, T>)
		{
			this->task.rethrow();
		}
		else
		{
			return std::move(task).result();
		}
	}
};

/**
	Lvalue reference task awaiter
*/
template<typename T>
struct LvTaskAwaiter
{
	Async::Task<T>& task;

	LvTaskAwaiter(Async::Task<T>& task_): task(task_)
	{}

	bool await_ready() const noexcept
	{
		return task.ready();
	}

	void await_suspend(std::coroutine_handle<> coro) noexcept
	{
		using namespace Runtime::Async;

		CoreTask& coreTask = static_cast<CoreTaskPtr&>(this->task).coreTask();
		coreTask.setContinuation({Scheduler::Invocation{std::move(coro)}, Scheduler::getCurrent()});
	}

	T await_resume()
	{
		if constexpr (std::is_same_v<void, T>)
		{
			this->task.rethrow();
		}
		else
		{
			return task.result();
		}
	}
};

/**
*/
struct DelayAwaiter
{
	const std::chrono::milliseconds delay;

	template<typename R, typename P>
	DelayAwaiter(std::chrono::duration<R, P> delay_): delay(std::chrono::duration_cast<std::chrono::milliseconds>(delay_))
	{}

	inline bool await_ready() const noexcept
	{
		return delay == std::chrono::milliseconds(0);
	}

	inline void await_suspend(std::coroutine_handle<> continuation) const noexcept
	{
		runtimeResumeAfter(delay, Async::Scheduler::getDefault(), std::move(continuation));
	}

	inline void await_resume() const noexcept
	{}
};


/**

*/
template<typename T>
struct TaskPromiseBase
{
	Async::TaskSource<T> taskSource;

	Async::Task<T> get_return_object()
	{
		return taskSource.getTask();
	}

	STD_CORO::suspend_never initial_suspend() const noexcept
	{
		return {};
	}

	STD_CORO::suspend_never final_suspend() const noexcept
	{
		return {};
	}

	void unhandled_exception() noexcept
	{
		taskSource.reject(std::current_exception());
	}

	template<typename U>
	static RvTaskAwaiter<U> await_transform(Async::Task<U>&& task) noexcept
	{
		return {std::move(task)};
	}

	template<typename U>
	static LvTaskAwaiter<U> await_transform(Async::Task<U>& task) noexcept
	{
		return {(task)};
	}

	static Async::SchedulerAwaiter await_transform(Async::Scheduler::Ptr scheduler) noexcept
	{
		return std::move(scheduler);
	}

	static Async::SchedulerAwaiter await_transform(Async::Scheduler::WeakPtr scheduler) noexcept
	{
		return scheduler.acquire();
	}

	template<typename Rep, typename Period>
	static DelayAwaiter await_transform(std::chrono::duration<Rep, Period> delay) noexcept
	{
		return delay;
	}
};


bool waitHelper(Async::CoreTaskPtr, std::optional<std::chrono::milliseconds>);

Async::Task<bool> whenAllHelper(Async::TaskContainerIterator, void* iteratorState, std::optional<std::chrono::milliseconds> timeout);

Async::Task<bool> whenAnyHelper(Async::TaskContainerIterator, void* iteratorState, std::optional<std::chrono::milliseconds> timeout);


} // namespace AsyncDetail

namespace Async {

template<typename T>
inline constexpr bool IsTask = IsTemplateOf<Task, T>;

/**
*/
template<typename T>
inline bool wait(Task<T>& task, std::optional<std::chrono::milliseconds> timeout = std::nullopt)
{
	return AsyncDetail::waitHelper(static_cast<CoreTaskPtr&>(task), timeout);
}


template<typename T>
inline T waitResult(Task<T> task)
{
	Assert(task);
	Verify(wait(task));

	return std::move(task).result();
}


template<typename T>
inline T waitResult(std::reference_wrapper<Task<T>> taskReference)
{
	decltype(auto) task = static_cast<Task<T>&>(taskReference);
	Assert(task);
	Verify(Async::wait(task));

	return task.result();
}

/**
*/
template<typename Container>
inline Task<bool> whenAll(Container& tasks, std::optional<std::chrono::milliseconds> timeout = std::nullopt)
{
	using namespace Runtime::AsyncDetail;
	using IteratorState = TaskContainerIteratorState<Container>;

	IteratorState state{tasks};
	return whenAllHelper(IteratorState::getIterator(), &state, timeout);
}

/**
*/
template<typename Container>
inline Task<bool> whenAny(Container& tasks, std::optional<std::chrono::milliseconds> timeout = std::nullopt)
{
	using namespace Runtime::AsyncDetail;
	using IteratorState = TaskContainerIteratorState<Container>;

	IteratorState state{tasks};
	return whenAnyHelper(IteratorState::getIterator(), &state, timeout);
}


template<typename T>
inline Task<bool> whenAll(std::initializer_list<Task<T>> tasks, std::optional<std::chrono::milliseconds> timeout = std::nullopt)
{
	using namespace Runtime::AsyncDetail;
	using IteratorState = TaskContainerIteratorState<decltype(tasks)>;

	IteratorState state{tasks};
	return whenAllHelper(IteratorState::getIterator(), &state, timeout);
}


template<typename T>
inline Task<bool> whenAny(std::initializer_list<Task<T>> tasks, std::optional<std::chrono::milliseconds> timeout = std::nullopt)
{
	using namespace Runtime::AsyncDetail;
	using IteratorState = TaskContainerIteratorState<decltype(tasks)>;

	IteratorState state{tasks};
	return whenAllHelper(IteratorState::getIterator(), &state, timeout);
}


template<typename F, typename ... Args,
	std::enable_if_t<IsTask<std::invoke_result_t<F, Args...>>, int> = 0>
// requires (IsTask<std::invoke_result_t<F, Args...>>)
std::invoke_result_t<F, Args...> run(F operation, Scheduler::Ptr scheduler, Args ... args)
{
	static_assert(IsTask<std::invoke_result_t<F, Args...>>);

	static_assert(std::is_invocable_v<F, Args...>, "Invalid functor. Arguments does not match expected parameters.");

	if (!scheduler)
	{
		scheduler = Scheduler::getDefault();
	}

	if (scheduler.get() != Scheduler::getCurrent().get())
	{
		co_await scheduler;
	}

	using TaskType = std::invoke_result_t<F, Args...>;

	// Scheduler::InvocationGuard invoke {*scheduler}; must be applied above,
	// because currently we are inside of schduler's invocation.
	TaskType task = operation(std::move(args)...);

	//{
	//	Scheduler::InvocationGuard invoke {*scheduler};
	//	task = operation(std::move(args)...);
	//}

	co_return (co_await task);
}


template<typename F, typename ... Args,
	std::enable_if_t<!IsTask<std::invoke_result_t<F, Args...>>, int> = 0>
// requires (IsTask<std::invoke_result_t<F, Args...>>)
Task<std::invoke_result_t<F, Args...>> run(F operation, Scheduler::Ptr scheduler, Args ... args)
{
	using Result = std::invoke_result_t<F, Args...>;

	static_assert(std::is_invocable_v<F, Args...>, "Invalid functor. Arguments does not match expected parameters.");

	if (!scheduler)
	{
		scheduler = Scheduler::getDefault();
	}

	if (scheduler.get() != Scheduler::getCurrent().get())
	{
		co_await scheduler;
	}

	using TaskType = std::invoke_result_t<F, Args...>;

	// Scheduler::InvocationGuard invoke {*scheduler}; must be applied above,
	// because currently we are inside of schduler's invocation.

	if constexpr (!std::is_same_v<Result, void>) {
		co_return operation(std::move(args)...);
	}
	else {
		operation(std::move(args)...);
	}

	//{
	//	Scheduler::InvocationGuard invoke {*scheduler};
	//	task = operation(std::move(args)...);
	//}
}



} // namespace Async
} // namespace Runtime



namespace STD_CORO {

template<typename T, typename ... Args>
struct coroutine_traits<Runtime::Async::Task<T>, Args...>
{
	struct promise_type : Runtime::AsyncDetail::TaskPromiseBase<T>
	{
		template<typename U>
		void return_value(U&& value)
		{
			static_assert(std::is_constructible_v<T, std::decay_t<U>>, "Invalid return value. Check co_return statement.");
			this->taskSource.resolve(std::forward<U>(value));
		}
	};
};


template<typename ... Args>
struct coroutine_traits<Runtime::Async::Task<void>, Args...>
{
	struct promise_type : Runtime::AsyncDetail::TaskPromiseBase<void>
	{
		void return_void()
		{
			this->taskSource.resolve();
		}
	};
};


} // namespace std
