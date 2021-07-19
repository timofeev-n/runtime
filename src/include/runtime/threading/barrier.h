#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>


namespace Runtime::Threading {

/**
* 
*/
class Barrier
{
public:

	Barrier(size_t total);

	Barrier(const Barrier&) = delete;

	void enter();

private:

	const size_t m_total;
	std::atomic_size_t m_counter{0};
	std::mutex m_mutex;
	std::condition_variable m_signal;
};


}
