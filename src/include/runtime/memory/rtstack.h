#pragma once

#include <runtime/memory/allocator.h>
#include <runtime/utils/preprocessor.h>

namespace Runtime {

class RtStackGuard
{
public:


	static Allocator& allocator();


	RtStackGuard();

	RtStackGuard(Kilobyte size);

	~RtStackGuard();

	RtStackGuard(const RtStackGuard&) = delete;

	RtStackGuard& operator = (const RtStackGuard&) = delete;


private:

	RtStackGuard* const m_prev = nullptr;
	Allocator::Ptr m_allocator;
	size_t m_top = 0;
};


Allocator& rtStack();

std::pmr::memory_resource* rtStackMemoryResource();


template<typename T>
using RtStackStdAllocator = StdAllocator<T, RtStackGuard>;

}


#define rtstack(...) const ::Runtime::RtStackGuard ANONYMOUS_VARIABLE_NAME(rtStack__) {__VA_ARGS__}

