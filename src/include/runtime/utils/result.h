#pragma once
#include <runtime/diagnostics/exception.h>
#include <runtime/utils/typeutility.h>
#include <runtime/meta/classinfo.h>
#include <runtime/compiler/coroutine.h>

#include <Core/Log.h>
#include <EngineAssert.h>

#include <array>
#include <exception>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>

// #define RUNTIME_RESULT_AWAIT


namespace Runtime {

template<typename T = void>
class Result;

struct Success
{};


constexpr inline Success success;

/**
*/
template<typename T = std::exception>
struct InplaceErrorTag
{};

/*
*/
struct InplaceResultTag
{};

/**
*/
template<typename T>
inline const InplaceErrorTag<T> InplaceError;

/**
*/
inline const InplaceResultTag InplaceResult;


/**
*/
struct ExceptionData
{
	static constexpr size_t MaxStoredTypesInfo = 5;

	std::exception_ptr exception;
	size_t typesInfoCount = 0;
	std::array<const std::type_info*, MaxStoredTypesInfo> typesInfo = {nullptr, nullptr, nullptr, nullptr, nullptr};

	ExceptionData(std::exception_ptr exception_): exception(exception_)
	{
		Assert(exception);
	}

	template<typename E,
		std::enable_if_t<std::is_base_of_v<std::exception, E>, int> = 0>
	// requires (std::is_base_of_v<std::exception, E>)
	ExceptionData(E&& exception): ExceptionData(std::make_exception_ptr(std::move(exception))) {
		using Types = typelist::AppendHead<Meta::ClassAllUniqueBase<E>, E>;

		InitializeExceptionTypes(*this, Types{});
	}

	template<typename E, 
		std::enable_if_t<std::is_base_of_v<std::exception, E>, int> = 0>
	// requires (std::is_base_of_v<std::exception, E>)
	bool hasException() const noexcept
	{
		const std::type_info& exceptionType = typeid(E);
		if (exceptionType == typeid(std::exception))
		{
			return true;
		}

		for (size_t i = 0; i < typesInfoCount; ++i)
		{
			if (*typesInfo[i] == exceptionType)
			{
				return true;
			}
		}

		try
		{
			Assert(this->exception);
			std::rethrow_exception(this->exception);
		}
		catch (const E&)
		{
			return true;
		}
		catch (...)
		{
		}

		return false;
	}

private:

	template<typename ... ExcType>
	static void InitializeExceptionTypes(ExceptionData& self, TypeList<ExcType...>) {

		const auto setExceptionType = [](ExceptionData& self, const std::type_info& type) {
			if (self.typesInfoCount < self.typesInfo.size()) {
				self.typesInfo[self.typesInfoCount++] = &type;
			}
		};

		(setExceptionType(self, typeid(ExcType)), ...);
	}
};


namespace Detail {

#ifdef RUNTIME_RESULT_AWAIT

template<typename>
struct ResultCoroPromiseBase;

template<typename>
struct ResultCoroPromise;

template<typename>
struct ResultCoroAwaiterRVRef;

template<typename>
struct ResultCoroAwaiterLVRef;

#endif

template<typename>
class ResultBase;


/**
*/
template<typename T>
class ResultBase
{
	static_assert(!std::is_same_v<T, std::exception_ptr>, "std::exception_ptr is not acceptable value type for Result<>");
	static_assert(!std::is_same_v<T, Result<T>*>, "Result* is not acceptable value type for Result<>");

protected:
	using ValueType = std::conditional_t<std::is_same_v<void, T>, bool, T>;
	using ResultProxy = Result<T>*;

public:
	using Data = std::variant<ResultProxy, ExceptionData, ValueType>;

	explicit operator bool () const
	{
		return std::holds_alternative<ValueType>(m_data);
	}

	template<typename E, typename ... Args,
		std::enable_if_t<std::is_base_of_v<std::exception, E> && std::is_constructible_v<ExceptionImplType<E>, Args...>, int> = 0>
	//requires std::is_base_of_v<std::exception, E> && std::is_constructible_v<ExceptionImplType<E>, Args...>
	ResultBase(InplaceErrorTag<E>, Args&& ... args): m_data(std::in_place_type<ExceptionData>, ExceptionImplType<E>{std::forward<Args>(args)...} )
	{
	}

	template<typename E,
		std::enable_if_t<std::is_base_of_v<std::exception, E>, int> = 0
		>
	//requires (std::is_base_of_v<std::exception, E>)
	ResultBase(E exception): m_data(std::in_place_type<ExceptionData>, std::move(exception)) {
	}

	ResultBase(std::exception_ptr exception): m_data(std::in_place_type<ExceptionData>, std::move(exception))
	{}

	ResultBase(ExceptionData exception): m_data(std::in_place_type<ExceptionData>, std::move(exception))
	{}

	ResultBase(ResultBase<T>&& other): m_data(std::move(other.m_data))
	{
		if (Result<T>* const proxy = getProxy(); proxy)
		{
			static_cast<ResultBase<T>*>(proxy)->m_data.template emplace<ResultProxy>(static_cast<Result<T>*>(this));
		}
	}

	ResultBase(const ResultBase& other): m_data(other.data)
	{
		Assert(!std::holds_alternative<ResultProxy>(m_data));
	}

	ResultBase<T>& operator = (const ResultBase<T>& r)
	{
		m_data = r.m_data;
		Assert(!std::holds_alternative<ResultProxy>(m_data));
		return *this;
	}

	ResultBase<T>& operator = (ResultBase<T>&& r)
	{
		m_data = std::move(r.m_data);
		Assert(!std::holds_alternative<ResultProxy>(m_data));
		return *this;
	}

	template<typename E = std::exception,
		std::enable_if_t<std::is_base_of_v<std::exception, E>, int> = 0>
	// requires (std::is_base_of_v<std::exception, E>)
	bool hasException() const noexcept
	{
		const ExceptionData* const exc = std::get_if<ExceptionData>(&m_data);
		return exc && exc->hasException<E>();
	}

	void rethrowIfException() const
	{
		if (const ExceptionData* const exc = std::get_if<ExceptionData>(&m_data); exc)
		{
			Assert(exc->exception);
			std::rethrow_exception(exc->exception);
		}
	}

	void ignore() const noexcept
	{
		if (this->hasException())
		{
			LOG_WARN("Ignoring Result<{0}> that holds an exception", typeid(T).name());
		}
	}

	ExceptionData err()
	{
		ExceptionData* const err = std::get_if<ExceptionData>(&m_data);
		Verify(err);
		ExceptionData temp = std::move(*err);
		m_data.template emplace<ResultProxy>(nullptr);
		return std::move(temp);
	}

protected:
	~ResultBase()
	{
		if(auto proxy = clearProxy(); proxy)
		{
			proxy->clearProxy();
		}
	}

	ResultBase() = default;

	ResultBase(Result<T>* proxy_): m_data(std::in_place_type<ResultProxy>, proxy_)
	{
		proxy_->m_data.template emplace<ResultProxy>(static_cast<Result<T>*>(this));
	}

	Result<T>* clearProxy()
	{
		if (!std::holds_alternative<ResultProxy>(this->m_data))
		{
			return nullptr;
		}

		Result<T>* proxy = nullptr;
		std::swap(std::get<ResultProxy>(this->m_data), proxy);
		return proxy;
	}

	Result<T>* getProxy() const
	{
		if (!std::holds_alternative<ResultProxy>(this->m_data))
		{
			return nullptr;
		}
		return std::get<ResultProxy>(this->m_data);
	}

	Data m_data;
};


} // namespace Detail




template<typename T>
class [[nodiscard]] Result final : public Detail::ResultBase<T>
{
public:

	using Detail::ResultBase<T>::ResultBase;
	using Detail::ResultBase<T>::operator =;
	using Detail::ResultBase<T>::operator bool;

	template<typename ... Args>
	Result(InplaceResultTag, Args&& ... args)
	{
		this->m_data.template emplace<T>(std::forward<Args>(args)...);
	}

	template<typename U,
		std::enable_if_t<std::is_same_v<T,U> && std::is_move_constructible_v<T>, int> = 0
	>
	Result(U&& value) //requires std::is_move_constructible_v<T>
	{
		this->m_data.template emplace<T>(std::move(value));
	}

	template<typename U,
		std::enable_if_t<std::is_same_v<T,U> && std::is_copy_constructible_v<U>, int> = 0
	>
	Result(const U& value) //requires std::is_copy_constructible_v<T>
	{
		this->m_data.template emplace<T>(value);
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T> && std::is_constructible_v<T, U>, int> = 0
	>
	//requires (!std::is_same_v<U,T> && std::is_constructible_v<T, U>)
	Result(U&& value)
	{
		this->m_data.template emplace<T>(std::forward<U>(value));
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T> && std::is_constructible_v<T, U>, int> = 0
	>
	//requires (!std::is_same_v<U,T> && std::is_constructible_v<T, U>)
	Result(Result<U>&& other)
	{
		constexpr bool ImplementMe = false;
		static_assert(ImplementMe, "Need to implement");
	}

	template<typename U,
		std::enable_if_t<!std::is_same_v<U,T> && std::is_assignable_v<T&, U>, int> = 0
	>
	//requires (!std::is_same_v<U,T> && std::is_assignable_v<T&, U>)
	Result& operator = (const Result<U>& other)
	{
		return *this;
	}

	const T& operator* () const &
	{
		this->rethrowIfException();
		return std::get<T>(this->m_data);
	}

	T& operator* () &
	{
		this->rethrowIfException();
		return std::get<T>(this->m_data);
	}

	T&& operator* () &&
	{
		this->rethrowIfException();
		return std::move(std::get<T>(this->m_data));
	}

	const T* operator -> () const
	{
		this->rethrowIfException();
		return &std::get<T>(this->m_data);
	}

	T* operator -> ()
	{
		this->rethrowIfException();
		return &std::get<T>(this->m_data);
	}

private:
	Result() = default;

#ifdef RUNTIME_RESULT_AWAIT

	Result(Result<T>* proxy_): Detail::ResultBase<T>(proxy_)
	{}


	friend struct Runtime::Detail::ResultCoroPromiseBase<T>;

	friend struct Runtime::Detail::ResultCoroPromise<T>;

	template<typename>
	friend struct Runtime::Detail::ResultCoroAwaiterRVRef;

	template<typename>
	friend struct Runtime::Detail::ResultCoroAwaiterLVRef;
#endif
};

/**
*/
template<>
class [[nodiscard]] Result<void> : public Detail::ResultBase<void>
{
public:
	using Detail::ResultBase<void>::ResultBase;
	using Detail::ResultBase<void>::operator =;
	using Detail::ResultBase<void>::operator bool;

	Result(Success)
	{
	 this->m_data.emplace<bool>(true);
	}

private:
	Result() = default;

#ifdef RUNTIME_RESULT_AWAIT
	friend struct Runtime::Detail::ResultCoroPromiseBase<void>;
	friend struct Runtime::Detail::ResultCoroPromise<void>;

	template<typename>
	friend struct Runtime::Detail::ResultCoroAwaiterRVRef;

	template<typename>
	friend struct Runtime::Detail::ResultCoroAwaiterLVRef;
#endif
};

template<typename T>
inline constexpr bool IsResult = Runtime::IsTemplateOf<Result, T>;

#ifdef RUNTIME_RESULT_AWAIT

namespace Detail {

template<typename T>
struct ResultCoroAwaiterRVRef
{
	Result<T> m_result;

	ResultCoroAwaiterRVRef(Result<T>&& result_): m_result(std::move(result_))
	{}

	bool await_ready() const noexcept
	{
		return static_cast<bool>(m_result);
	}

	template<typename U>
	void await_suspend(std::coroutine_handle<ResultCoroPromise<U>> coro_) noexcept;


	T await_resume() noexcept
	{
		return *std::move(m_result);
	}
};


template<typename T>
struct ResultCoroAwaiterLVRef
{
	const Result<T>& m_result;

	ResultCoroAwaiterLVRef(const Result<T>& result_): m_result(result_)
	{}

	bool await_ready() const noexcept
	{
		return static_cast<bool>(m_result);
	}

	template<typename U>
	void await_suspend(std::coroutine_handle<ResultCoroPromise<U>> coro_) noexcept ;

	T await_resume() const noexcept
	{
		return *m_result;
	}
};


template<typename T>
struct ResultCoroPromiseBase
{
	Result<T> result;

	std::suspend_never initial_suspend() noexcept
	{
		return {};
	}

	std::suspend_always final_suspend() noexcept
	{
		return {};
	}

	Result<T> get_return_object()
	{
		return Result<T>{&this->result};
	}

	void unhandled_exception() noexcept
	{
		Result<T>* const proxy = this->result.clearProxy();
		proxy->m_data.template emplace<ExceptionData>(std::current_exception());
	}

	template<typename U>
	static auto await_transform(Result<U>&& result_)
	{
		return ResultCoroAwaiterRVRef<U>{std::move(result_)};
	}

	template<typename U>
	static auto await_transform(const Result<U>& result_)
	{
		return ResultCoroAwaiterLVRef<U>{result_};
	}

	template<typename U>
	static decltype(auto) await_transform(U&& value)
	{
		static_assert(Runtime::IsTemplateOf<Runtime::Result ,U>, "Only Result<> can be used for co_await operation");
		return std::forward<U>(value);
	}
};



/**
*/
template<typename T>
struct ResultCoroPromise : ResultCoroPromiseBase<T>
{
	template<typename U>
	void return_value(U&& value)
	{
		static_assert(std::is_constructible_v<T,U>, "Invalid return value type: it can not be converted to coroutine target type");

		Result<T>* const proxy = this->result.clearProxy();
		Assert(proxy);

		proxy->m_data.template emplace<T>(std::forward<U>(value));
	}
};

template<>
struct ResultCoroPromise<void> : public ResultCoroPromiseBase<void>
{
	void return_void()
	{
		Result<>* const proxy = this->result.clearProxy();
		Assert(proxy);

		proxy->m_data.template emplace<bool>(true);
	}
};


template<typename T>
template<typename U>
void ResultCoroAwaiterRVRef<T>::await_suspend(std::coroutine_handle<ResultCoroPromise<U>> coro) noexcept
{
	Result<U>* targetResult = coro.promise().result.clearProxy();
	Assert(targetResult);
	const ExceptionData* const exceptionData = std::get_if<ExceptionData>(&this->m_result.m_data);
	Assert(exceptionData);
	targetResult->m_data.template emplace<ExceptionData>(*exceptionData);
	coro.destroy();
}


template<typename T>
template<typename U>
void ResultCoroAwaiterLVRef<T>::await_suspend(std::coroutine_handle<ResultCoroPromise<U>> coro) noexcept
{
	Result<U>* targetResult = coro.promise().result.clearProxy();
	Assert(targetResult);
	ExceptionData* const exceptionData = std::get_if<ExceptionData>(&this->m_result);
	Assert(exceptionData);
	targetResult->template emplace<ExceptionData>(*exceptionData);
	coro.destroy();
}


} // namespace Detail

#endif

} // namespace Runtime

#ifdef RUNTIME_RESULT_AWAIT

namespace STD_CORO {

template<typename T, typename ... Args>
struct coroutine_traits<Runtime::Result<T>, Args...>
{
	using promise_type = Runtime::Detail::ResultCoroPromise<T>;
};

}

#endif
