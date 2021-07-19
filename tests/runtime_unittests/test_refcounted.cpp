#include "pch.h"
#include <runtime/com/refcounted.h>



using namespace Runtime;
using namespace testing;


//template<Runtime::RefCounted RC>
template<typename RC>
class Test_RefCounted : public testing::Test
{
public:
	RC m_counter;
};


#if 0
using RCTypes = testing::Types<ConcurrentRC, SingleThreadRC, StrictSingleThreadRC>;
TYPED_TEST_SUITE(Test_RefCounted, RCTypes);



TYPED_TEST(Test_RefCounted, InitiallyHasOneRef)
{
	ASSERT_THAT(refsCount(this->m_counter), Eq(1));
}


TYPED_TEST(Test_RefCounted, AddRef)
{
	ASSERT_THAT(this->m_counter.addRef(), Eq(1));
	ASSERT_THAT(this->m_counter.addRef(), Eq(2));
}


TYPED_TEST(Test_RefCounted, RemoveRef)
{
	ASSERT_THAT(this->m_counter.addRef(), Eq(1));
	ASSERT_THAT(this->m_counter.removeRef(), Eq(2));
	ASSERT_THAT(this->m_counter.removeRef(), Eq(1));
}

#endif

TEST(Test_ConcurrentRC, Multithread)
{
	using Counter = Runtime::ConcurrentRC;

	constexpr size_t ThreadsCount = 20;
	constexpr size_t IterationsCount = 10000;

	Counter counter;


	const auto runInThread = [&counter, ThreadsCount, IterationsCount](auto f)
	{
		std::vector<std::thread> threads;

		for (size_t i = 0; i < ThreadsCount; ++i)
		{
			threads.emplace_back([&counter, IterationsCount, &f]
			{
				for (size_t x = 0; x < IterationsCount; ++x)
				{
					f(counter);
				}
			});
		}

		for (auto& t : threads)
		{
			t.join();
		}
	};


	runInThread([](Counter& counter)
	{
		counter.addRef();
	});

	ASSERT_THAT(refsCount(counter), Eq(ThreadsCount * IterationsCount + 1));


	runInThread([](Counter& counter)
	{
		counter.removeRef();
	});

	ASSERT_THAT(refsCount(counter), Eq(1));
}
