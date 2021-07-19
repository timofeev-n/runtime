#include "pch.h"
#include "transport.h"
#include "runtime/runtime/disposableruntimeguard.h"
#include "runtime/runtime/internal/runtimeinternal.h"

#include "runtime/utils/intrusivelist.h"
#include "runtime/remoting/socketaddress.h"


using namespace Runtime::Async;
using namespace Runtime::Remoting;

namespace Runtime::Network {

namespace {

decltype(auto) registries()
{
	static std::forward_list<TransportFactoryRegistry> transportFactories {
		RegisterTcpTransport(),
		RegisterPipeTransport()
	};

	return (transportFactories);
}

}

ComPtr<TransportClient> TransportFactory::createClient(std::string_view protocol) noexcept
{
	for (const TransportFactoryRegistry& registry : registries())
	{
		if (!registry.clientFactory)
		{
			continue;
		}

		if (auto client = registry.clientFactory(protocol); client)
		{
			return client;
		}
	}

	return {};
}


ComPtr<TransportServer> TransportFactory::createServer(std::string_view protocol) noexcept
{
	for (const TransportFactoryRegistry& registry : registries()) {
		if (!registry.serverFactory) {
			continue;
		}

		if (auto server = registry.serverFactory(protocol)) {
			return server;
		}
	}

	return {};
}


TransportFactoryRegistry::TransportFactoryRegistry(TransportFactoryRegistry::ClientFactory clientFactory_, TransportFactoryRegistry::ServerFactory serverFactory_)
	: clientFactory(clientFactory_)
	, serverFactory(serverFactory_)
{
	Assert(clientFactory || serverFactory);

	//registries().push_back(*this);
}

//-----------------------------------------------------------------------------

Task<Server::Ptr> Server::listen(std::string addressString, std::optional<unsigned> backlog)
{
	auto [protocol, address] = SocketAddress::parseAddressString(addressString);

	SWITCH_RUNTIME_SCHEDULER

	auto server = TransportFactory::createServer(protocol);
	if (!server)
	{
		co_return Server::Ptr{};
	}

	co_await server->listen(address, backlog);

	co_return std::move(server);
}


Task<Stream::Ptr> Client::connect(std::string addressString, Expiration expiration)
{
	auto [protocol, address] = SocketAddress::parseAddressString(addressString);

	SWITCH_RUNTIME_SCHEDULER

	auto client = TransportFactory::createClient(protocol);
	if (!client)
	{
		co_return Stream::Ptr{};
	}

	rtdisposable(*client);

	do
	{
		if (co_await client->connect(address))
		{
			co_return std::move(client);
		}
	}
	while (!expiration.isExpired());

	std::string message = Core::Format::format("Connection:({}) was not established during specified timeout or cancelled.", address);
	throw Excpt(NetworkException, std::move(message));
	
	co_return Stream::Ptr{};
}


} // namespace Runtime::Network
