#include "pch.h"
#include "testnetworkbase.h"
#include <runtime/network/utils/tcptablesnapshot.h>
#include <runtime/memory/rtstack.h>


using namespace Runtime;
using namespace Runtime::Async;

//-----------------------------------------------------------------------------
void Test_Network_Base::resetRuntime()
{
	m_runtime.reset();
}


std::vector<std::string> Test_Network_Base::addresses()
{
	using namespace Core;

	const auto tcpPortBusy = [](int port, const Network::TcpEntryHandle* handles, size_t count) -> bool
	{
		for (size_t i = 0; i < count; ++i)
		{
			if (const Network::TcpTableEntry entry {handles[i]}; entry.localPort == port)
			{
				return true;
			}
		}
		return false;
	};

	const auto chooseFreeTcpPort = [tcpPortBusy](unsigned portStart) -> unsigned
	{
		return Network::GetTcpTableSnapshot([&tcpPortBusy, portStart](const Network::TcpEntryHandle* handles, size_t count) -> unsigned
		{
			unsigned port = portStart;

			while (tcpPortBusy(port, handles, count))
			{
				++port;
			}
			
			return port;
		});
	};

	const unsigned port = chooseFreeTcpPort(5770);
	constexpr std::string_view PipeChannelName = ".\\playrixruntime-test-channel";

	return {
		Format::format("tcp://:{}", port),
		Format::format("ipc://{}", PipeChannelName)
		//Format::format("ws+tcp://:{}", port),
		//Format::format("ws+ipc://{}", PipeChannelName)
	};
}


Task<BytesBuffer> Test_Network_Base::readEos(Runtime::Network::Stream::Ptr stream)
{
	BytesBuffer result;

	do
	{
		auto inboundBuffer = co_await stream->read();
		if (inboundBuffer.size() == 0)
		{
			break;
		}

		result += std::move(inboundBuffer);
	}
	while(true);

	co_return result;
}


Task<BytesBuffer> Test_Network_Base::readCount(Network::Stream::Ptr stream, size_t bytesCount)
{
	BytesBuffer buffer;

	while (buffer.size() < bytesCount)
	{
		BytesBuffer frame = co_await stream->read();
		buffer += std::move(frame);
	}

	co_return buffer;
}

//-----------------------------------------------------------------------------
std::string Test_Network_Default::address() const
{
	return GetParam();
}
