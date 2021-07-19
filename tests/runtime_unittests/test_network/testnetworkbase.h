#pragma once
#include "pch.h"
#include "../helpers/runtimescopeguard.h"
#include <runtime/runtime/runtime.h>
#include <runtime/network/server.h>
#include <runtime/network/client.h>
#include <runtime/network/networkexception.h>
#include <runtime/memory/rtstack.h>

class Test_Network_Base
{
public:

	static std::vector<std::string> addresses();

	static Runtime::Async::Task<Runtime::BytesBuffer> readEos(Runtime::Network::Stream::Ptr stream);

	static Runtime::Async::Task<Runtime::BytesBuffer> readCount(Runtime::Network::Stream::Ptr stream, size_t);

protected:

	virtual std::string address() const = 0;

	void resetRuntime();

private:

	RuntimeScopeGuard m_runtime;
	rtstack(Runtime::Megabyte(1));
};



class Test_Network_Default : public Test_Network_Base, public testing::TestWithParam<std::string>
{
protected:

	std::string address() const override;
};

