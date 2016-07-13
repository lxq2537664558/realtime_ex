#pragma once
#include <condition_variable>
#include <mutex>
#include <vector>

#include "libBaseCommon/noncopyable.h"

namespace core
{
	struct	SMessagePacket
	{
		uint8_t		nType;
		void*		pData;
		uint32_t	nDataSize;
	};

	class CMessageQueue :
		public base::noncopyable
	{
	public:
		CMessageQueue();
		virtual ~CMessageQueue();

		bool					init(bool bWait);
		bool					pushMessagePacket(const SMessagePacket& sMessagePacket);
		void					popMessagePacket(std::vector<SMessagePacket>& vecMessagePacket);

	private:
		SMessagePacket*			m_pMessageQueue;
		int32_t					m_nQueueBegin;
		int32_t					m_nQueueEnd;
		int32_t					m_nQueueCapacity;

		bool					m_bWait;
		std::mutex				m_lock;
		std::condition_variable	m_cond;
	};
}