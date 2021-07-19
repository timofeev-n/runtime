#pragma once

#include <runtime/diagnostics/exceptionguard.h>

#include <type_traits>
#include <tuple>
#include <utility>

#ifdef _WIN32
#include <iphlpapi.h>
#include <winsock.h>
#endif


namespace Runtime::Network {

#ifdef _WIN32
using TcpEntryHandle = MIB_TCPROW;
#endif

enum class TcpState
{
	Closed,
	Listen,
	Unknown

};

struct TcpTableEntry
{
	unsigned localAddr = 0;
	unsigned localPort = 0;
	unsigned remoteAddr = 0;
	unsigned remotePort = 0;
	TcpState state = TcpState::Unknown;

	TcpTableEntry();

	TcpTableEntry(const TcpEntryHandle& handle);
};




void GetTcpTableSnapshotHelper(void (* callback)(const TcpEntryHandle* entries, size_t count, void*) noexcept, void*);


template<typename Callable, typename ... Args,
	std::enable_if_t<std::is_invocable_v<Callable, const TcpEntryHandle*, size_t, Args ...>, int> = 0>
//requires (std::is_invocable_v<Callable, const TcpEntryHandle*, size_t, Args ...>)
auto GetTcpTableSnapshot(Callable callable, Args ... args) -> std::invoke_result_t<Callable, const TcpEntryHandle*, size_t, Args ...>
{
	static_assert(std::is_invocable_v<Callable, const TcpEntryHandle*, size_t, Args ...>, "Invalid functor");

	using Result = std::invoke_result_t<Callable, const TcpEntryHandle*, size_t, Args ...>;

	Result result{};

	struct CallbackData
	{
		Result* const resultPtr;
		Callable& callable;
		std::tuple<Args&...> args;
	};

	CallbackData data = {&result, callable, std::tie(args...)};

	static auto callbackWrapper = [](const TcpEntryHandle* entries, size_t count, void* ptr) noexcept {
		CallbackData& data = *reinterpret_cast<CallbackData*>(ptr);
		*data.resultPtr = std::apply(data.callable, std::tuple_cat(std::tuple{entries, count}, data.args));
	};

	GetTcpTableSnapshotHelper([](const TcpEntryHandle* entries, size_t count, void* ptr) noexcept {
		callbackWrapper(entries, count, ptr);
	}, &data);



	return result;
}

}
