#include "pch.h"
#include "runtime/threading/barrier.h"

namespace Runtime::Threading {

Barrier::Barrier(size_t total): m_total(total) {
	Assert(m_total > 0);
}


void Barrier::enter() {
	const size_t maxExpectedCounter = m_total - 1;
	const size_t counter = m_counter.fetch_add(1);

	Assert(counter < m_total);

	if (counter == maxExpectedCounter) {
		m_signal.notify_all();

		return;
	}

	std::unique_lock lock(m_mutex);

	m_signal.wait(lock, [this] {
		return m_counter.load() == m_total;
	});
}

}
