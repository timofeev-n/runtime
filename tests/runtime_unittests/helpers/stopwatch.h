#pragma once
#include <chrono>

class StopWatch
{
public:
	using CountT = decltype(std::chrono::milliseconds{}.count());

	StopWatch();

	std::chrono::milliseconds timePassed() const;

	CountT ms() const;

private:

	const std::chrono::system_clock::time_point m_timePoint;
};

