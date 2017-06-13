#pragma once

#include <mutex>
#include <atomic>

#include "noncopyable.h"

namespace base
{
	// ����������ǲ���ƽ�ģ����Ҳ�������ģ�Ҫ�����ƽ�����⣬����ʹ��ticket lock
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
					_mm_pause();	// �൱��nop������Ϊ�˱���
				}
			} while (this->m_lock.exchange(true, std::memory_order_acquire));

			// ��ֱ��һ��while (exchange) ��Ϊ�˷�ֹ������ľ�������ʱ������������໥֮���cache line ˢ��
		}

		void unlock()
		{
			this->m_lock.store(false, std::memory_order_release);
		}

	private:
		std::atomic<bool> m_lock;
	};
}