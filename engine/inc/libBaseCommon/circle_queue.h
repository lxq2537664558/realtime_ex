#pragma once
#include "noncopyable.h"
#include "debug_helper.h"

namespace base
{
	template<class T, bool FIX>
	class CCircleQueue :
		public noncopyable
	{
	public:
		CCircleQueue(uint32_t nCap);
		~CCircleQueue();

		bool		send(const T& sValue);
		bool		recv(T& sValue);
		uint32_t	size() const;
		bool		empty() const;

	protected:
		T*		m_pQueue;
		int32_t	m_nQueueBegin;
		int32_t	m_nQueueEnd;
		int32_t	m_nQueueCapacity;
#ifdef _DEBUG
		int32_t	m_nSize;
#endif
	};
}

#include "circle_queue.inl"