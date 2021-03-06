#include "pch.h"
#include "stopwatch.h"


StopWatch::StopWatch(): m_timePoint(std::chrono::system_clock::now())
{}

std::chrono::milliseconds StopWatch::timePassed() const {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now() - m_timePoint);
}

StopWatch::CountT StopWatch::ms() const {
	return this->timePassed().count();
}
