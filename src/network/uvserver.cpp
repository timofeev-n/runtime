#include "pch.h"
#include "uvserver.h"
#include "runtime/network/networkexception.h"
#include "runtime/runtime/internal/runtimeinternal.h"
#include "runtime/runtime/internal/uvutils.h"
#include "runtime/diagnostics/exceptionguard.h"
#include "runtime/com/comclass.h"



namespace Runtime::Network {

using namespace Runtime::Async;


UvServer::~UvServer()
{
	this->doDispose(true);
}


void UvServer::dispose()
{
	this->doDispose(false);
}


void UvServer::doDispose(bool destructed) noexcept
{
	if (const bool alreadyDisposed = m_isDisposed.exchange(true); alreadyDisposed)
	{
		return;
	}

	const auto closeServer = [](UvServer& this_)
	{
		Assert(RuntimeCore::isRuntimeThread());

		this_.m_server.close();
		this_.resolveAccept({});
	};


	if (RuntimeCore::isRuntimeThread())
	{
		closeServer(*this);
		return;
	}

	if (destructed)
	{
		auto task = [](UvServer& this_, auto closeServer) -> Task<>
		{
			co_await RuntimeInternal::scheduler();
			closeServer(this_);

		}(*this, closeServer);
		
		Async::wait(task);
	}
	else
	{
		[](UvServer& this_, auto closeServer) -> Task<>
		{
			KeepRC(this_);
			co_await RuntimeInternal::scheduler();
			closeServer(this_);
		}
		(*this, closeServer).detach();
	}
}


Task<Stream::Ptr> UvServer::accept()
{
	const std::lock_guard lock {m_mutex};

	if (!m_server)
	{
		return Task<Stream::Ptr>::makeResolved(nothing);
	}

	if (!m_disposableGuard)
	{
		m_disposableGuard.emplace(*this);
	}

	Assert2(!m_acceptAwaiter, "::accept operation already processed");

	if (!m_inboundConnections.empty())
	{
		Stream::Ptr stream = std::move(m_inboundConnections.front());
		m_inboundConnections.pop_front();
		return Task<Stream::Ptr>::makeResolved(std::move(stream));
	}

	m_acceptAwaiter = {};
	return m_acceptAwaiter.getTask();
}


void UvServer::resolveAccept(Stream::Ptr stream)
{
	Assert(!stream || m_acceptAwaiter);
	if (m_acceptAwaiter)
	{
		TaskSource<Stream::Ptr> awaiter = std::move(m_acceptAwaiter);

		if (stream)
		{
			awaiter.resolve(std::move(stream));
		}
		else
		{
			awaiter.resolve(nothing);
		}
	}
}


Task<> UvServer::listen(std::string_view address, std::optional<unsigned> optionalBacklog)
{
	const auto listenCallback = [](uv_stream_t* server, int status) noexcept
	{
		Assert(server);
		if (!server->data)
		{
			return;
		}

		DEBUG_NOEXCEPT_Guard
		{
			UvServer& this_ = *reinterpret_cast<UvServer*>(server->data);

			const std::lock_guard lock{this_.m_mutex};

			if (status != 0)
			{
				this_.resolveAccept({});
				return;
			}

			// Actual uv_accept should not be delayed and stream must be created here:
			// 1. can call uv_accept and create actual Stream only when called Server::accept;
			// 2. if callee will not call Server::accept for each inbound connection there is possibility that UV will not close sockets for this inbound connections
			//		and because connection actually established and socket remaining alive remote client will hang forewer on read operation
			//		i.e. Client::connect finished with success and subsequent ::read() will never be finished.
			UvStreamHandle streamHandle = this_.createClientHandle();
			if (const auto code = uv_accept(server, streamHandle); code != 0)
			{
				LOG_INFO("Fail to accept inbound stream: ", uvErrorMessage(code));
				return ;
			}

			Stream::Ptr stream = UvStream::create(std::move(streamHandle));
			if (this_.m_acceptAwaiter)
			{
				this_.resolveAccept(std::move(stream));
			}
			else
			{
				this_.m_inboundConnections.emplace_back(std::move(stream));
			}
		}; // noexcept guard
	};


	m_server = co_await this->bind(address);

	Assert(m_server);

	uv_handle_set_data(m_server, this);

	constexpr int DefaultBacklogSize = 60;
	const int backlog = (optionalBacklog && *optionalBacklog > 0) ? static_cast<int>(*optionalBacklog) : DefaultBacklogSize;

	UV_THROW_ON_ERROR(uv_listen(m_server, backlog, listenCallback))
}

}
