#include "pch.h"
#include "coretaskimpl.h"

#include "runtime/async/core/coretask.h"
#include "runtime/async/core/coretasklinkedlist.h"
#include "runtime/threading/event.h"
//#include "runtime/diagnostics/runtimecheck.h"
#include "runtime/utils/scopeguard.h"

using namespace Runtime::Async;
using namespace Runtime::AsyncDetail;


namespace Runtime {

namespace Async {

#pragma region CoreTaskLinkedList

//-----------------------------------------------------------------------------

bool operator == (const CoreTaskLinkedList::iterator& iter1, const CoreTaskLinkedList::iterator& iter2)
{
	return iter1.m_task == iter2.m_task;
}

bool operator != (const CoreTaskLinkedList::iterator& iter1, const CoreTaskLinkedList::iterator& iter2)
{
	return iter1.m_task != iter2.m_task;
}

CoreTaskLinkedList::iterator& CoreTaskLinkedList::iterator::operator++()
{
	Assert2(m_task, "Task iterator is not dereferencable");
	m_task = static_cast<CoreTaskImpl*>(m_task)->next();

	return *this;
}

CoreTaskLinkedList::iterator CoreTaskLinkedList::iterator::operator++(int)
{
	Assert2(m_task, "Task iterator is not dereferencable");
	
	iterator temp{*this};
	this->operator++();
	return temp;
}

CoreTask* CoreTaskLinkedList::iterator::operator*() const
{
	Assert2(m_task, "Task iterator is not dereferencable");
	return m_task;
}

CoreTaskLinkedList::iterator::iterator(CoreTask* task_): m_task(task_)
{
}

//-----------------------------------------------------------------------------
CoreTaskLinkedList::CoreTaskLinkedList(TaskContainerIterator taskIterator, void* iteratorState)
{
	CoreTaskImpl* last = nullptr;

	do
	{
		CoreTaskPtr next = taskIterator(iteratorState);
		if (!next)
		{
			break;
		}

		CoreTaskImpl* const task = static_cast<CoreTaskImpl*>(getCoreTask(next));
		Assert(task);
		task->addRef();

		if (!m_head)
		{
			m_head = task;
		}

		if (last)
		{
			Assert(!last->next());
			last->setNext(task);
		}

		last = task;
		++m_size;
	}
	while (true);
}

CoreTaskLinkedList::~CoreTaskLinkedList()
{
	CoreTaskImpl* next = static_cast<CoreTaskImpl*>(m_head);

	while (next)
	{
		CoreTaskImpl* const current = next;
		next = next->next();

		current->setNext(nullptr);
		current->setReadyCallback(nullptr, nullptr);
		current->release();
	}
}

CoreTaskLinkedList::iterator CoreTaskLinkedList::begin()
{
	return {m_head};
}


CoreTaskLinkedList::iterator CoreTaskLinkedList::end()
{
	return iterator {};
}


size_t CoreTaskLinkedList::size() const
{
#ifndef NDEBUG
	const CoreTaskImpl* next = static_cast<const CoreTaskImpl*>(m_head);
	size_t counter = 0;
	while (next)
	{
		++counter;
		next = next->next();
	}

	Assert(m_size == counter);
#endif

	return m_size;
}


bool CoreTaskLinkedList::empty() const
{
	return m_head == nullptr;
}

#pragma endregion

} // namespace Async

namespace AsyncDetail {

//-----------------------------------------------------------------------------

bool waitHelper(CoreTaskPtr taskPtr, std::optional<std::chrono::milliseconds> timeout) {
	CoreTask* const task = getCoreTask(taskPtr);
	if (task->ready())
	{
		return true;
	}

	SCOPE_Leave {
		task->setReadyCallback(nullptr, nullptr);
	};

	Threading::Event signal;
	task->setReadyCallback([](void* ptr, void*) noexcept
	{
		auto& signal = *reinterpret_cast<Threading::Event*>(ptr);
		signal.set();
	}, &signal);

	return signal.wait(timeout);
}


Task<bool> whenAllHelper(TaskContainerIterator taskIterator, void* iteratorState, std::optional<std::chrono::milliseconds> timeout) {
	CoreTaskLinkedList tasks{taskIterator, iteratorState};

	const size_t size = tasks.size();
	if (size == 0)
	{
		co_return true;
	}

	struct AwaiterState
	{
		std::atomic_size_t counter;
		TaskSource<> taskSource;

	} awaiterState { /*.counter = */ size };

	for (CoreTask* const task : tasks)
	{
		task->setReadyCallback([](void* ptr, void*) noexcept
		{
			auto& awaiterState = *reinterpret_cast<AwaiterState*>(ptr);
			if (awaiterState.counter.fetch_sub(1) == 1)
			{
				awaiterState.taskSource.resolve();
			}
		}, &awaiterState);
	}

	co_await awaiterState.taskSource.getTask();

	co_return true;
}


Task<bool> whenAnyHelper(TaskContainerIterator taskIterator, void* iteratorState, std::optional<std::chrono::milliseconds> timeout) {
	CoreTaskLinkedList tasks{taskIterator, iteratorState};

	if (tasks.empty() || std::any_of(tasks.begin(), tasks.end(), [](const CoreTask* task) {return task->ready(); }))
	{
		co_return true;
	}

	TaskSource<> taskSource;

	for (CoreTask* const task : tasks)
	{
		task->setReadyCallback([](void* ptr, void*) noexcept
		{
			auto& taskSource = *reinterpret_cast<TaskSource<>*>(ptr);
			taskSource.resolve();
		}, &taskSource);
	}

	co_await taskSource.getTask();

	co_return true;
}


} // namespace AsyncDetail
} // namespace Runtime
