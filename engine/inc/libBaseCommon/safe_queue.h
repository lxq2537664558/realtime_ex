#pragma once
#include <atomic>

#include "base_common.h"
#include "noncopyable.h"

namespace base
{
	// 这是一个在单生产者单消费者下 线程安全的队列, 
	// 这个队列不足的地方是一旦队列满了，需要在别的地方暂存，
	// 势必需要内存拷贝，但是一般来说这样的情况不会很多，除非
	// 消费者实在是太慢了，并且内存拷贝相比于锁竞争开销要小很多
	template<typename T, uint32_t nSize>
	class safe_queue
	{
	public:
		enum { nMaxCount = nSize };

	public:
		safe_queue()
		{
			this->m_nPushCount = 0;
			this->m_nPopCount = 0;
		};

		~safe_queue(){};

		bool push(const T& data)
		{
			if (this->m_nPushCount - this->m_nPopCount.load(std::memory_order_acquire) >= nMaxCount)
				return false;

			this->m_zArray[this->m_nPushCount%nMaxCount] = data;

			this->m_nPushCount.store(this->m_nPushCount + 1, std::memory_order_release);
			return true;
		}

		bool get(T*& data)
		{
			if (this->m_nPushCount - this->m_nPopCount.load(std::memory_order_acquire) >= nMaxCount)
				return false;

			data = &this->m_zArray[this->m_nPushCount%nMaxCount];
		}

		bool push()
		{
			if (this->m_nPushCount - this->m_nPopCount.load(std::memory_order_acquire) >= nMaxCount)
				return false;

			this->m_nPushCount.store(this->m_nPushCount + 1, std::memory_order_release);
			return true;
		}

		bool pop(T& data)
		{
			if (this->m_nPushCount.load(std::memory_order_acquire) == this->m_nPopCount)
				return false;

			data = this->m_zArray[this->m_nPopCount%nMaxCount];

			this->m_nPopCount.store(this->m_nPopCount + 1, std::memory_order_release);
			return true;
		}

	private:
		T						m_zArray[nMaxCount];
		std::atomic<uint64_t>	m_nPushCount;
		std::atomic<uint64_t>	m_nPopCount;
	};
}