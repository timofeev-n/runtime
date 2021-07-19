#pragma once
#include "runtime/com/ianything.h"
#include "runtime/network/client.h"
#include "runtime/network/server.h"
#include "runtime/runtime/internal/uvutils.h"

#include <string_view>

namespace Runtime::Network {


struct ABSTRACT_TYPE TransportClient : virtual Stream
{
	DECLARE_CLASS_BASE(Stream)

	virtual Async::Task<bool> connect(std::string_view address) = 0;
};


struct ABSTRACT_TYPE TransportServer : Server
{
	DECLARE_CLASS_BASE(Server)

	virtual Async::Task<> listen(std::string_view address, std::optional<unsigned> backlog) = 0;
};


struct TransportFactory
{
	static ComPtr<TransportClient> createClient(std::string_view protocol) noexcept;

	static ComPtr<TransportServer> createServer(std::string_view protocol) noexcept;
};


struct TransportFactoryRegistry
{
	using ClientFactory = ComPtr<TransportClient> (*) (std::string_view);
	using ServerFactory = ComPtr<TransportServer> (*) (std::string_view);


	const ClientFactory clientFactory = nullptr;
	const ServerFactory serverFactory = nullptr;

	TransportFactoryRegistry(ClientFactory, ServerFactory);
};


TransportFactoryRegistry RegisterTcpTransport();
TransportFactoryRegistry RegisterPipeTransport();



}
