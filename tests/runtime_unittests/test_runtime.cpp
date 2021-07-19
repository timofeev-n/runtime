#include "pch.h"
#include "helpers/runtimescopeguard.h"
#include <runtime/runtime/runtime.h>
#include <runtime/runtime/internal/runtimeinternal.h>
#include <runtime/threading/event.h>


using namespace Runtime;
using namespace testing;




TEST(Test_Runtime, Test1)
{
	constexpr size_t DefaultWorksCount = 100;
	constexpr size_t PoolWorksCount = 500;

	size_t counter = 0;
	std::atomic_size_t poolCounter = 0;

	{
		RuntimeScopeGuard runtimeGuard;

		for (size_t i = DefaultWorksCount; i > 0; --i ) {

			RuntimeInternal::scheduler()->schedule([](void* p1, void* counterPtr) noexcept
			{
				++(*reinterpret_cast<size_t*>(counterPtr));

				if (reinterpret_cast<ptrdiff_t>(p1) == 1) {
					RuntimeCore::instance().stop();
				}

			}, reinterpret_cast<void*>(i), &counter);
		}

		for (size_t i = 0; i < PoolWorksCount; ++i ) {
			RuntimeCore::instance().poolScheduler()->schedule([](void*, void* poolCounterPtr) noexcept {
				reinterpret_cast<std::atomic_size_t*>(poolCounterPtr)->fetch_add(1);
			}, nullptr, &poolCounter);
		}
	}

	ASSERT_THAT(counter, Eq(DefaultWorksCount));
	ASSERT_THAT(poolCounter, Eq(PoolWorksCount));
}
