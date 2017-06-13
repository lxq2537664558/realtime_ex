#pragma once

#include <mutex>
#include <atomic>

#include "noncopyable.h"

namespace base
{
	// 这个自旋锁是不公平的，并且不可重入的，要解决公平性问题，考虑使用ticket lock
	class spin_lock :
		public noncopyable
	{
	public:
		spin_lock()
		{
			this->m_lock.store(false, std::memory_order_relaxed);
		}

		void lock()
		{
			do
			{
				while (this->m_lock.load(std::memory_order_relaxed));
				{
					_mm_pause();	// 相当于nop，这里为了避退
				}
			} while (this->m_lock.exchange(true, std::memory_order_acquire));

			// 不直接一个while (exchange) 是为了防止多个核心竞争锁的时候，引起大量的相互之间的cache line 刷新
		}

		void unlock()
		{
			this->m_lock.store(false, std::memory_order_release);
		}

	private:
		std::atomic<bool> m_lock;
	};
}