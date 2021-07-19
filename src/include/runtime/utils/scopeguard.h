#pragma once
#include <runtime/utils/preprocessor.h>

#include <exception>
#include <type_traits>


namespace Runtime::Detail {


//template<typename F>
//concept ScopeGuardCallback = std::is_invocable_r_v<void, F>;


/// <summary>
///
/// </summary>
template<typename F>
struct ScopeGuard_OnLeave
{
	F callback;

	ScopeGuard_OnLeave(F callback_): callback(std::move(callback_))
	{}

	~ScopeGuard_OnLeave() noexcept(noexcept(callback())) {
		callback();
	}
};


template<typename F>
struct ScopeGuard_OnSuccess
{
	F callback;
	const int exceptionsCount = std::uncaught_exceptions();

	ScopeGuard_OnSuccess(F callback_): callback(std::move(callback_))
	{}

	~ScopeGuard_OnSuccess() noexcept(noexcept(callback()))
	{
		if (exceptionsCount == std::uncaught_exceptions())
		{
			callback();
		}
	}
};


template<typename F>
struct ScopeGuard_OnFail
{
	F callback;
	const int exceptionsCount = std::uncaught_exceptions();

	ScopeGuard_OnFail(F callback_): callback(std::move(callback_))
	{
		static_assert(noexcept(callback()), "Failure scope guard must be noexcept(true)");
	}

	~ScopeGuard_OnFail() noexcept
	{
		if (exceptionsCount < std::uncaught_exceptions())
		{
			callback();
		}
	}
};


struct ExprBlock
{
	template<typename F>
	inline constexpr friend decltype(auto) operator + (const ExprBlock&, F f)
	{
		static_assert(std::is_invocable_v<F>);
		return f();
	}
};

}


#define SCOPE_Fail\
	::Runtime::Detail::ScopeGuard_OnFail ANONYMOUS_VARIABLE_NAME(onScopeFailure__) = [&]() noexcept

#define SCOPE_Success\
	::Runtime::Detail::ScopeGuard_OnSuccess ANONYMOUS_VARIABLE_NAME(onScopeSuccess__) = [&]

#define SCOPE_Leave\
	::Runtime::Detail::ScopeGuard_OnLeave ANONYMOUS_VARIABLE_NAME(onScopeLeave__) = [&]

#define EXPR_Block ::Runtime::Detail::ExprBlock{} + [&]()
