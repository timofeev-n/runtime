#pragma once
#include <runtime/utils/preprocessor.h>

#include <EngineAssert.h>

#include <exception>


namespace Runtime::Detail {

struct NoExceptGuard
{
	const wchar_t* const func;
	const wchar_t* const source;
	const int line;

	NoExceptGuard(const wchar_t* func_, const wchar_t* source_, int line_): func(func_), source(source_), line(line_)
	{}

	template<typename F>
	inline friend void operator + (NoExceptGuard guard, F f) noexcept
	{
		try
		{
			f();
		}
		catch (const std::exception& exception)
		{
			const auto message = Core::Format::format("Unexpected exception: {}", exception.what());
			Halt(message);
		}
		catch (...)
		{
			Halt("Unexpected non typed exception");
		}
	}
};

}

#define NOEXCEPT_Guard ::Runtime::Detail::NoExceptGuard{WFUNCTION, WFILE, __LINE__} + [&]()


#if !defined(NDEBUG)
#define DEBUG_NOEXCEPT_Guard ::Runtime::Detail::NoExceptGuard{WFUNCTION, WFILE, __LINE__} + [&]()
#else
#define DEBUG_NOEXCEPT_Guard
#endif

