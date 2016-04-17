#pragma once

#include <mutex>
#include <atomic>

#include "noncopyable.h"

namespace base
{
	class spin_mutex :
		public noncopyable
	{
	public:
		spin_mutex()
		{
			this->m_lock.clear();
		}

		void lock()
		{
			while (m_lock.test_and_set(std::memory_order_acquire));
		}
		void unlock()
		{
			m_lock.clear(std::memory_order_release);
		}

	private:
		std::atomic_flag m_lock;
	};
}