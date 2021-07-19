#include "pch.h"
#include "runtime/com/comclass.h"
#include "runtime/memory/allocator.h"
#include "runtime/memory/pageallocator.h"
#include "runtime/threading/critical_section.h"
#include "runtime/utils/scopeguard.h"



namespace Runtime {

namespace {


struct NoLock
{
	void lock() noexcept {}
	void unlock() noexcept {}
};

} // namespace


/**
* 
*/
class Pool
{
public:

	Pool(size_t poolSize_, size_t blockSize_ ): m_memory(poolSize_), m_blockSize(blockSize_)
	{}

	size_t size() const
	{
		return m_memory.size();
	}

	size_t blockSize() const
	{
		return m_blockSize;
	}

	void* allocate()
	{
		if (m_freeBlocksBegin)
		{
			Block* const block = reinterpret_cast<Block*>(m_freeBlocksBegin);
			m_freeBlocksBegin = block->next;
			return block;
		}

		return m_memory.alloc(m_blockSize);
	}

	void free(void* ptr)
	{
		Assert(contains(ptr));

		reinterpret_cast<Block*>(ptr)->next = m_freeBlocksBegin;
		m_freeBlocksBegin = ptr;
	}

	bool contains(void* ptr) const
	{
		return m_memory.contains(ptr);
	}

	size_t availableBytes() const
	{
	}

private:

	struct Block
	{
		void* next;
	};

	PageAllocator m_memory;
	const size_t m_blockSize;
	void* m_freeBlocksBegin = nullptr;
};


/**
* 
*/
template<typename Mutex>
class FixedSizePoolAllocatorImpl final : public Allocator
{
	COMCLASS_(Allocator)

public:

	FixedSizePoolAllocatorImpl(size_t blockGranularity_, size_t blockMaxSize_): m_blockGranularity(blockGranularity_), m_blockMaxSize(blockMaxSize_)
	{
		Assert(isPOT(m_blockGranularity));
	}

	~FixedSizePoolAllocatorImpl()
	{
		//LOG_debug_("Total pools ({0})", m_pools.size())
		//for (auto& pool : m_pools)
		//{
		//	LOG_debug_(" * [{0}], size = {1} bytes", pool.blockSize(), pool.size() )
		//}
	}


	void* realloc(void* prevPtr, size_t size, std::optional<size_t> alignment) override
	{
		Assert(!alignment || (isPOT(*alignment) && *alignment <= sizeof(std::max_align_t)));

		const size_t blockSize = alignedSize(size, m_blockGranularity);
		if (blockSize > m_blockMaxSize)
		{
			return crtAllocator()->realloc(prevPtr, blockSize, std::nullopt);
		}

		lock_(m_mutex);

		if (prevPtr)
		{
			auto pool = std::find_if(m_pools.begin(), m_pools.end(), [prevPtr](const Pool& pool)
			{
				return pool.contains(prevPtr);
			});

			if (pool == m_pools.end()) // ptr was allocated by CRT allocator.
			{
				return crtAllocator()->realloc(prevPtr, size);
			}

			const size_t oldSize = pool->blockSize();
			if (size <= oldSize)
			{ // already fit into allocated block
				return prevPtr;
			}

			SCOPE_Leave {
				this->free(prevPtr, std::nullopt);
			};

			void* const newPtr = this->realloc(nullptr, size, alignment);
			if (newPtr) // if newPtr is null, then default/crt allocator already fail to allocate
			{
				memcpy(newPtr, prevPtr, oldSize);
			}

			return newPtr;
		} // << realloc


		if (size == 0)
		{
			return nullptr;
		}

		for (auto& pool : m_pools)
		{
			if (pool.blockSize() == blockSize)
			{
				if (void* const ptr = pool.allocate(); ptr )
				{
					return ptr;
				}
			}
		}

		constexpr size_t MinPoolSize = AllocationGranuarity;
		constexpr size_t OptimalBlocksPerPool = 256;
	
		size_t maxBlocksPerPool = 0;

		for (const Pool& pool : m_pools)
		{
			if (pool.blockSize() == blockSize)
			{
				maxBlocksPerPool = std::max(maxBlocksPerPool, pool.size() / blockSize);
			}
		}

		const size_t blocksPerPool = std::max(OptimalBlocksPerPool, maxBlocksPerPool * 2);
		const size_t optimalPoolSize = blockSize * blocksPerPool;
		const size_t poolSize = alignedSize(std::max(optimalPoolSize, MinPoolSize), AllocationGranuarity);

		Pool& pool = m_pools.emplace_front(poolSize, blockSize);
		return pool.allocate();
	}

	void free(void* ptr, std::optional<size_t> size) override
	{
		if (!ptr)
		{
			Assert(!size || *size == 0);
			return;
		}

		lock_(m_mutex);

		if (auto pool = std::find_if(m_pools.begin(), m_pools.end(), [ptr](const Pool& pool) { return pool.contains(ptr); }); pool != m_pools.end())
		{
			pool->free(ptr);
		}
		else
		{
			crtAllocator()->free(ptr, size);
		}
	}

private:

	const size_t m_blockGranularity;
	const size_t m_blockMaxSize;
	Mutex m_mutex;
	std::list<Pool> m_pools;
};


Allocator::Ptr createPoolAllocator(PoolAllocatorConfig config)
{
	constexpr size_t DefaultAlignment = sizeof(std::max_align_t);
	constexpr size_t MinGranularity = DefaultAlignment;
	
	const size_t targetGranularity = std::max(alignedSize(config.granularity, DefaultAlignment), MinGranularity);

	using ConcurrentAllocator = FixedSizePoolAllocatorImpl<Threading::CriticalSection>;
	using SingleThreadAllocator = FixedSizePoolAllocatorImpl<NoLock>;

	if (config.concurrent)
	{
		return Com::createInstance<ConcurrentAllocator, Allocator>(targetGranularity, config.blockMaxSize);
	}

	return Com::createInstance<SingleThreadAllocator, Allocator>(targetGranularity, config.blockMaxSize);
}


} // namespace Runtime
