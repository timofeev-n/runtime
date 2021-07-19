#pragma once
#include "uvstream.h"
#include "transport.h"

#include "runtime/network/server.h"
#include "runtime/runtime/disposableruntimeguard.h"
#include "runtime/threading/critical_section.h"


namespace Runtime::Network {

class ABSTRACT_TYPE UvServer : public TransportServer
{
	DECLARE_CLASS_BASE(TransportServer)

public:

	~UvServer();

	void dispose() override;

	Async::Task<Stream::Ptr> accept() override;

	Async::Task<> listen(std::string_view, std::optional<unsigned> backlog) override;

protected:

	virtual Async::Task<UvStreamHandle> bind(std::string_view) = 0;

	virtual UvStreamHandle createClientHandle() = 0;

private:

	void resolveAccept(Stream::Ptr);

	void doDispose(bool destructed) noexcept;

	UvStreamHandle m_server;
	Async::TaskSource<Stream::Ptr> m_acceptAwaiter = nothing;
	std::list<Stream::Ptr> m_inboundConnections;
	Threading::CriticalSection m_mutex;
	std::optional<DisposableRuntimeGuard> m_disposableGuard;
	std::atomic_bool m_isDisposed = false;
};

}
