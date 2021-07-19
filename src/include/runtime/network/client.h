#pragma once

#include <runtime/network/stream.h>
#include <runtime/utils/cancellationtoken.h>

#include <string>


namespace Runtime::Network {


struct Client
{
	static Async::Task<Stream::Ptr> connect(std::string address, Expiration = Expiration::never());
};

}
