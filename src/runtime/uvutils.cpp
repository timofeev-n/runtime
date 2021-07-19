#include "pch.h"
#include "runtime/runtime/runtime.h"
#include "runtime/runtime/internal/uvutils.h"
#include "runtime/runtime/internal/runtimeinternal.h"
#include "runtime/runtime/internal/runtimeallocator.h"

#include "runtime/async/task.h"
#include "runtime/threading/event.h"
#include "runtime/threading/critical_section.h"
#include "runtime/utils/intrusivelist.h"


namespace Runtime {

namespace {

inline void closeAndFreeUvHandle(uv_handle_t* handle)
{
	Assert(RuntimeCore::isRuntimeThread());
	Assert(uv_is_closing(handle) == 0);

	uv_handle_set_data(handle, nullptr);



	if (handle->type == UV_TCP || handle->type == UV_NAMED_PIPE)
	{
		uv_stream_t* const stream = reinterpret_cast<uv_stream_t*>(handle);
		if (uv_is_writable(stream) != 0 && uv_is_active(handle) != 0)
		{
			uv_shutdown_t* const request = reinterpret_cast<uv_shutdown_t*>(runtimePoolAllocate(sizeof(uv_shutdown_t)));

			const auto shutdownCode = uv_shutdown(request, stream, [](uv_shutdown_t* request, int status) noexcept
			{
				uv_handle_t* const handle = reinterpret_cast<uv_handle_t*>(request->handle);

				runtimePoolFree(request);

				uv_close(handle, freeUvHandle);
			});

			if (shutdownCode == 0)
			{
				return;
			}

			runtimePoolFree(request);

		}
	}


	uv_close(handle, freeUvHandle);
}


} // namespace


uv_handle_t* allocateUvHandle(uv_handle_type what) noexcept
{
	Assert(RuntimeCore::isRuntimeThread());

	const auto handleSize = uv_handle_size(what);
	Assert2(handleSize > 0, Core::Format::format("Unknown uv handle ({}) size", what));

	void* const mem = runtimePoolAllocate(handleSize);
	auto handle = reinterpret_cast<uv_handle_t*>(mem);

#if !defined(NDEBUG) || defined(DEBUG)
	memset(handle, 0, handleSize);
#endif

	return handle;
}

void freeUvHandle(uv_handle_t* handle) noexcept
{
	Assert(handle);
	runtimePoolFree(handle);
}

std::string uvErrorMessage(int code, std::string_view customMessage) noexcept
{
	if (code == 0)
	{
		return std::string{};
	}

	const char* const errName = uv_err_name(code);
	if (!customMessage.empty())
	{
		return Core::Format::format("{}:{}", errName, customMessage);
	}

	const char* const errStr = uv_strerror(code);
	return Core::Format::format("{}:{}", errName, errStr);
}

uv_handle_type uvHandleType(const std::type_info& type)
{
	if (type == typeid(uv_async_t)) {
		return UV_ASYNC;
	}
	else if (type == typeid(uv_timer_t)) {
		return UV_TIMER;
	}
	else if (type == typeid(uv_tcp_t)) {
		return UV_TCP;
	}
	else if (type == typeid(uv_pipe_t)) {
		return UV_NAMED_PIPE;
	}
	else if (type == typeid(uv_tty_t)) {
		return UV_TTY;
	}

	return UV_UNKNOWN_HANDLE;
}


namespace Detail {

UvHandleBase::UvHandleBase(uv_handle_type handleType)
{
	if (handleType != UV_UNKNOWN_HANDLE)
	{
		m_handle = allocateUvHandle(handleType);
		Assert(m_handle);
	}
}

UvHandleBase::~UvHandleBase()
{
	close();
}

void UvHandleBase::close()
{
	if (!m_handle)
	{
		return;
	}

	uv_handle_t* const handle = m_handle;
	m_handle = nullptr;

	if (RuntimeCore::isRuntimeThread())
	{
		closeAndFreeUvHandle(handle);
	}
	else
	{
		auto task = [](uv_handle_t* handle) -> Async::Task<>
		{
			co_await RuntimeInternal::scheduler();
			closeAndFreeUvHandle(handle);

		}(handle);

		task.detach();
	}
}

UvHandleBase::operator bool () const
{
	return m_handle != nullptr;
}

void UvHandleBase::setData(void* data_)
{
	Assert(m_handle);
	uv_handle_set_data(m_handle, data_);
}

void* UvHandleBase::data() const
{
	Assert(m_handle);
	return uv_handle_get_data(m_handle);
}

bool UvHandleBase::isAssignable(const std::type_info& type, uv_handle_type handleType)
{
	if (handleType == UV_UNKNOWN_HANDLE)
	{
		return false;
	}

	if (type == typeid(uv_handle_t))
	{
		return true;
	}

	if (type == typeid(uv_stream_t))
	{
		return handleType == UV_TCP || handleType == UV_NAMED_PIPE || handleType == UV_TTY;
	}

	return uvHandleType(type) == handleType;
}

}}
