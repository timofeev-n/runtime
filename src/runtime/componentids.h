#pragma once
#include <string_view>

namespace Runtime {

struct ComponentIds
{
	static constexpr std::string_view DefaultScheduler = "default_scheduler";
	static constexpr std::string_view PoolScheduler = "pool_scheduler";
	static constexpr std::string_view TimerManager = "timer_manager";
	//static constexpr std::string_view Network = "network";
};


}
