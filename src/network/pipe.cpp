#include "pch.h"
#include "uvserver.h"
#include "networkutility.h"
#include "runtime/utils/strings.h"
#include "runtime/remoting/socketaddress.h"
#include "runtime/runtime/internal/runtimeinternal.h"
#include "runtime/diagnostics/exceptionguard.h"
#include "runtime/com/comclass.h"


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4250)
#endif


namespace Runtime::Network {

using namespace Runtime::Async;


class PipeServer final : public UvServer
{
	COMCLASS_(UvServer)

private:

	Task<UvStreamHandle> bind(std::string_view address) override
	{
		const auto [host, service] = Remoting::SocketAddress::parseIpcAddress(address);
		const std::string path = makePipeFilePath(host, service);

		UvHandle<uv_pipe_t> pipe;
		UV_RUNTIME_CHECK(uv_pipe_init(RuntimeInternal::uv(), pipe, 0))
		UV_THROW_ON_ERROR(uv_pipe_bind(pipe, path.c_str()))

		return Task<UvStreamHandle>::makeResolved(std::move(pipe));
	}

	UvStreamHandle createClientHandle() override
	{
		UvHandle<uv_pipe_t> pipe;
		UV_RUNTIME_CHECK(uv_pipe_init(RuntimeInternal::uv(), pipe, 0))
		return pipe;
	}
};


class PipeClient final : public virtual UvStream, public virtual TransportClient
{
	COMCLASS_(UvStream, TransportClient)

public:
	
	PipeClient(): UvStream(createPipe())
	{}


private:

	Task<bool> connect(std::string_view address) override
	{
		const auto callback = [](uv_connect_t* connect, int status) noexcept
		{
			Assert(connect);

			DEBUG_NOEXCEPT_Guard
			{
				auto& connectAwaiter = *reinterpret_cast<TaskSource<bool>*>(connect->data);

				if (status == 0)
				{
					connectAwaiter.resolve(true);
				}
				else if (status == UV_ENOENT)
				{
					connectAwaiter.resolve(false);
				}
				else
				{
					auto message = uvErrorMessage(status);
					connectAwaiter.reject(Excpt(NetworkException, message));
				}
			};
		};

		const auto [host, service] = Remoting::SocketAddress::parseIpcAddress(address);
		const std::string path = makePipeFilePath(host, service);

		THROW_IF_DISPOSED(*this)

		TaskSource<bool> connectAwaiter;
		uv_connect_t connectRequest;
		connectRequest.data = &connectAwaiter;

		uv_pipe_connect(&connectRequest, this->stream().as<uv_pipe_t>(), path.c_str(), callback);

		const bool connected = co_await connectAwaiter.getTask();
		co_return connected;
	}

	static inline UvStreamHandle createPipe()
	{
		UvHandle<uv_pipe_t> pipe;
		UV_RUNTIME_CHECK(uv_pipe_init(RuntimeInternal::uv(), pipe, 0))
		return pipe;
	}
};

/* -------------------------------------------------------------------------- */

TransportFactoryRegistry RegisterPipeTransport() {

	return {
		[](std::string_view protocol) -> ComPtr<TransportClient> {
			if (!Strings::icaseEqual(protocol, "ipc")) {
				return ComPtr<TransportClient>{};
			}

			return Com::createInstance<PipeClient, TransportClient>();
		},

		[](std::string_view protocol) {
			if (!Strings::icaseEqual(protocol, "ipc")) {
				return ComPtr<TransportServer>{};
			}

			return Com::createInstance<PipeServer, TransportServer>();
		}
	};
}

}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
