#pragma once

#include <runtime/runtime/runtime.h>
#include <runtime/utils/preprocessor.h>
#include <runtime/utils/typeutility.h>
#include <runtime/network/networkexception.h>

#include <EngineAssert.h>

#include <uv.h>
#include <string>
#include <type_traits>

namespace Runtime {

template<typename>
class UvHandle;

namespace Detail {

class UvHandleBase
{
public:

	UvHandleBase() = default;

	UvHandleBase(uv_handle_type);

	UvHandleBase(const UvHandleBase&) = delete;

	~UvHandleBase();

	void close();

	explicit operator bool () const;

	void setData(void*);

	void* data() const;

protected:

	static bool isAssignable(const std::type_info& type, uv_handle_type handleType);


	uv_handle_t* m_handle = nullptr;

	template<typename>
	friend class UvHandle;

};

} // namespace Detail

uv_handle_t* allocateUvHandle(uv_handle_type what) noexcept;

void freeUvHandle(uv_handle_t*) noexcept;

std::string uvErrorMessage(int code, std::string_view customMessage = {}) noexcept;

uv_handle_type uvHandleType(const std::type_info& handleType);

template<typename T>
inline uv_handle_t* asUvHandle(T* handle)
{
	static_assert(sizeof(T) >= sizeof(uv_handle_t));
	Assert(handle);
	
	return reinterpret_cast<uv_handle_t*>(handle);
}
/**
*/
template<typename T = uv_handle_t>
class UvHandle : public Detail::UvHandleBase
{
	template<typename U>
	static constexpr bool CrossAssignable = !std::is_same_v<U,T> && (AnyOf<T, uv_handle_t, uv_stream_t> || AnyOf<U, uv_handle_t, uv_stream_t>);

public:

	UvHandle(): Detail::UvHandleBase(uvHandleType(typeid(T)))
	{}

	UvHandle(UvHandle&& other)
	{
		std::swap(m_handle, other.m_handle);
		Assert(!other.m_handle);
	}

	template<typename U,
		std::enable_if_t<CrossAssignable<U>, int> = 0>
	UvHandle(UvHandle<U>&& other) // requires CrossAssignable<U>
	{
		if (other.m_handle)
		{
			uv_handle_t* const otherHandle = static_cast<UvHandleBase&>(other).m_handle;

			Assert(UvHandleBase::isAssignable(typeid(T), otherHandle->type));
			m_handle = asUvHandle(other.m_handle);
			other.m_handle = nullptr;
		}
	}

	UvHandle<T>& operator = (UvHandle&& other)
	{
		close();
		Assert(!m_handle);
		std::swap(m_handle, other.m_handle);

		return *this;
	}

	template<typename U,
		std::enable_if_t<CrossAssignable<U>, int> = 0>
	UvHandle<T>& operator = (UvHandle<U>&& other) //requires CrossAssignable<U>
	{
		close();
		if (other.m_handle)
		{
			Assert(UvHandleBase::isAssignable(typeid(T), other.m_handle->type));
			m_handle = asUvHandle(other.m_handle);
			other.m_handle = nullptr;
		}

		return *this;
	}

	T* operator -> () const
	{
		Assert(m_handle);
		Assert(m_handle->type == UV_ASYNC || RuntimeCore::isRuntimeThread());

		return reinterpret_cast<T*>(m_handle);
	}

	operator T* () const // requires (!std::is_same_v<T, uv_handle_t>)
	{
		static_assert(!std::is_same_v<T, uv_handle_t>);

		Assert(m_handle);
		Assert(m_handle->type == UV_ASYNC || RuntimeCore::isRuntimeThread());

		return reinterpret_cast<T*>(m_handle);
	}

	operator uv_handle_t* () const
	{
		Assert(m_handle);
		Assert(m_handle->type == UV_ASYNC || RuntimeCore::isRuntimeThread());

		return m_handle;
	}

	template<typename U>
	U* as() const
	{
		Assert(m_handle);
		Assert(UvHandleBase::isAssignable(typeid(U), m_handle->type));

		return reinterpret_cast<U*>(m_handle);
	}

	template<typename> friend 
	class UvHandle;
};

using UvStreamHandle = UvHandle<uv_stream_t>;

} // namespace Runtime


#define UV_RUNTIME_CHECK(expression)\
	if (const int resultCode = (expression); resultCode != 0) { \
		const auto customMessage = ::Runtime::uvErrorMessage(resultCode);\
		Halt(customMessage);\
	}

#define UV_THROW_ON_ERROR(expression)\
	if (const int resultCode = expression; resultCode != 0) { \
		const auto customMessage = ::Runtime::uvErrorMessage(resultCode);\
		throw Excpt(::Runtime::Network::NetworkException, customMessage);\
	}\

