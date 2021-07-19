#pragma once

#include <runtime/async/scheduler.h>
#include <runtime/async/core/coretask.h>
#include <runtime/utils/scopeguard.h>
#include <runtime/utils/nothing.h>
#include <EngineAssert.h>

#include <exception>
#include <type_traits>

namespace Runtime::Async {

template<typename T>
class TaskBase;

template<typename T>
class TaskSourceBase;

template<typename T = void>
class Task;

template<typename T = void>
class TaskSource;

} // namespace Runtime::Async


namespace Runtime::AsyncDetail {

template<typename T>
struct TaskData;

template<>
struct TaskData<void>
{
	bool taskDetached = false;
	bool taskGivenOut = false;
};

template<typename T>
struct TaskData : TaskData<void>
{
	std::optional<T> result;
};


template<typename T>
class TaskStateHolder : public Async::CoreTaskPtr
{
public:
	TaskStateHolder(const TaskStateHolder<T>&) = delete;
	TaskStateHolder(TaskStateHolder<T>&&) noexcept = default;
	TaskStateHolder& operator = (const TaskStateHolder&) = delete;
	TaskStateHolder& operator = (TaskStateHolder&&) = default;


	bool ready() const
	{
		Assert2(static_cast<bool>(*this), "Task is stateless");
		return this->coreTask().ready();
	}

protected:

	TaskStateHolder() = default;

	TaskStateHolder(Async::CoreTaskPtr&& coreTask_): Async::CoreTaskPtr(std::move(coreTask_))
	{}

	const TaskData<T>& taskData() const &
	{
		const void* const ptr = this->coreTask().data();
		return *reinterpret_cast<const TaskData<T>*>(ptr);
	}

	TaskData<T>& taskData() &
	{
		void* const ptr = this->coreTask().data();
		return *reinterpret_cast<TaskData<T>*>(ptr);
	}

	TaskData<T>&& taskData() &&
	{
		void* const ptr = this->coreTask().data();
		return std::move(*reinterpret_cast<TaskData<T>*>(ptr));
	}
};


} // namespace Runtime::AsyncDetail


namespace Runtime {
namespace Async {

#pragma region Task

/**
*/
template<typename T>
class TaskBase : public AsyncDetail::TaskStateHolder<T>
{
public:
	using ValueType = T;

	static Task<T> makeRejected(std::exception_ptr exception) noexcept;

	template<typename E,
		std::enable_if_t<std::is_base_of_v<std::exception, E>, int> = 0>
	// requires (std::is_base_of_v<std::exception, E>)
	static Task<T> makeRejected(E exception) noexcept;


	TaskBase() = default;
	TaskBase(const TaskBase<T>&) = delete;
	TaskBase(TaskBase<T>&&) noexcept = default;
	TaskBase<T>& operator = (const TaskBase<T>&) = delete;
	TaskBase<T>& operator = (TaskBase<T>&&) = default;


	void rethrow() const
	{
		this->coreTask().rethrow();
	}

	std::exception_ptr exception() const
	{
		return this->coreTask().exception();
	}

	bool rejected() const
	{
		return static_cast<bool>(this->coreTask().exception());
	}
	
	Task<T>& detach() &
	{
		Assert2(!this->taskData().taskDetached, "Task already detached");
		this->taskData().taskDetached = true;

		return static_cast<Task<T>&>(*this);
	}

	Task<T>&& detach() &&
	{
		Assert2(!this->taskData().taskDetached, "Task already detached");
		this->taskData().taskDetached = true;

		return std::move(static_cast<Task<T>&>(*this));
	}

protected:

	TaskBase(Async::CoreTaskPtr&& coreTask_): AsyncDetail::TaskStateHolder<T>(std::move(coreTask_))
	{}

	~TaskBase()
	{
		if (std::uncaught_exceptions() > 0 || !static_cast<bool>(*this))
		{
			return;
		}

#if !defined(NDEBUG)
		[[maybe_unused]] const bool taskReadyOrDetached = this->ready() || this->taskData().taskDetached || this->coreTask().hasContinuation();
		Assert2(taskReadyOrDetached, "Not finished Async::Task<> is leaving its scope. Use co_await or set continuation.");
#endif
	}
};

/**
*/
template<typename T>
class [[nodiscard]] Task final : public TaskBase<T>
{
public:
	template<typename ... Args>
	static Task<T> makeResolved(Args&&...) ;//requires(std::is_constructible_v<T, Args...>);

	Task() = default;
	Task(const Task<T>&) = delete;
	Task(Task<T>&&) = default;
	Task<T>& operator = (const Task<T>&) = delete;
	Task<T>& operator = (Task<T>&&) = default;


	T result() & // requires(std::is_copy_assignable_v<T>)
	{
		static_assert(std::is_copy_assignable_v<T>);

		Assert2(this->ready(), ::Core::Format::format("Task<{}> is not ready", typeid(T).name()));
		this->rethrow();

		Assert(this->taskData().result);
		return *this->taskData().result;
	}

	T result() &&
	{
		Assert2(this->ready(), Core::Format::format("Task<{}> is not ready", typeid(T).name()));

		SCOPE_Leave {
			this->reset();
		};

		this->rethrow();
		Assert(this->taskData().result);
		return std::move(*this->taskData().result);
	}

	T operator * () &
	{
		return this->result();
	}

	T operator * () &&
	{
		return this->result();
	}

private:
	Task(Async::CoreTaskPtr&& coreTask_): TaskBase<T>(std::move(coreTask_))
	{}

	friend class TaskSourceBase<T>;
};

/**
*/
template<>
class [[nodiscard]] Task<void> final : public TaskBase<void>
{
public:
	static Task<void> makeResolved();

	Task() = default;
	Task(const Task<>&) = delete;
	Task(Task<>&&) = default;
	Task<>& operator = (const Task<>&) = delete;
	Task<>& operator = (Task<>&&) = default;


	void result() const &
	{
		this->rethrow();
	}

	void result() &&
	{
		SCOPE_Leave {
			this->reset();
		};

		this->rethrow();
	}

private:
	Task(Async::CoreTaskPtr&& coreTask_): TaskBase<void>(std::move(coreTask_))
	{}

	friend class TaskSourceBase<void>;
};

#pragma endregion


#pragma region TaskSource

/**
*/
template<typename T>
class TaskSourceBase : public AsyncDetail::TaskStateHolder<T>
{
public:

	TaskSourceBase(Nothing) {}
	TaskSourceBase(TaskSourceBase<T>&&) = default;
	TaskSourceBase(const TaskSourceBase<T>&) = delete;
	TaskSourceBase& operator = (const TaskSourceBase<T>&) = delete;
	TaskSourceBase& operator = (TaskSourceBase<T>&&) = default;

	bool reject(std::exception_ptr exception) noexcept
	{
		return this->coreTask().tryReject(std::move(exception));
	}

	template<typename E>
	// requires (std::is_base_of_v<std::exception, E>)
	bool reject(E exception) noexcept {
		static_assert(std::is_base_of_v<std::exception, E>);

		return this->coreTask().tryReject(std::make_exception_ptr(std::move(exception)));
	}

	Task<T> getTask()
	{
		Assert2(!this->taskData().taskGivenOut, Core::Format::format("Task<{}> already takedout from source", typeid(T).name()));

		this->taskData().taskGivenOut = true;
		Async::CoreTaskPtr coreTask = static_cast<Async::CoreTaskPtr&>(*this);
		return Task<T>{std::move(coreTask)};
	}

protected:

	TaskSourceBase()
		: AsyncDetail::TaskStateHolder<T>{createCoreTask<AsyncDetail::TaskData<T>>(ComPtr<Allocator>{})}
	{}

	virtual ~TaskSourceBase()
	{
		if (static_cast<bool>(*this) && !this->ready())
		{
			this->reject(std::exception("Task source destructed, but result is not set"));
		}
	}
};

/**
*/
template<typename T>
class TaskSource final : public TaskSourceBase<T>
{
public:
	using TaskSourceBase<T>::TaskSourceBase;

	template<typename ... Args>
	bool resolve(Args&& ... args) //requires(std::is_constructible_v<T, Args...>)
	{
		static_assert(std::is_constructible_v<T, Args...>);

		return this->coreTask().tryResolve([&](CoreTask::Rejector&) noexcept
		{
			this->taskData().result.emplace(std::forward<Args>(args)...);
		});
	}
};

/**
*/
template<>
class TaskSource<void> final : public TaskSourceBase<void>
{
public:
	using TaskSourceBase<void>::TaskSourceBase;

	bool resolve()
	{
		return this->coreTask().tryResolve();
	}
};

#pragma endregion


template<typename T>
Task<T> TaskBase<T>::makeRejected(std::exception_ptr exception) noexcept
{
	TaskSource<T> taskSource;
	taskSource.setException(std::move(exception));
	return taskSource.getTask();
}

template<typename T>
template<typename E, std::enable_if_t<std::is_base_of_v<std::exception, E>, int>>
//requires (std::is_base_of_v<std::exception, E>)
Task<T> TaskBase<T>::makeRejected(E exception) noexcept {

	static_assert(std::is_base_of_v<std::exception, E>);

	TaskSource<T> taskSource;
	taskSource.setException(std::move(exception));
	return taskSource.getTask();
}

template<typename T>
template<typename ... Args>
Task<T> Task<T>::makeResolved(Args&&...args) // requires(std::is_constructible_v<T, Args...>)
{
	static_assert(std::is_constructible_v<T, Args...>);

	TaskSource<T> taskSource;
	taskSource.resolve(std::forward<Args>(args)...);
	return taskSource.getTask();
}

inline Task<void> Task<void>::makeResolved() {
	TaskSource<> taskSource;
	taskSource.resolve();
	return taskSource.getTask();
}


} // namespace Async
} // namespace Runtime
