#pragma once
#include <atomic>

#include "base_common.h"
#include "noncopyable.h"

namespace base
{
	// ����һ���ڵ������ߵ��������� �̰߳�ȫ�Ķ���, 
	// ������в���ĵط���һ���������ˣ���Ҫ�ڱ�ĵط��ݴ棬
	// �Ʊ���Ҫ�ڴ濽��������һ����˵�������������ܶ࣬����
	// ������ʵ����̫���ˣ������ڴ濽�����������������ҪС�ܶ�
	// �����ײ�ܶ�����Ҳ����̫��
	template<class T, uint32_t nSize>
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