#include "stdafx.h"
#include "message_queue.h"

#include "libBaseCommon/debug_helper.h"

#define _DEFAULT_MESSAGE_QUEUE 10240

namespace core
{
	CMessageQueue::CMessageQueue()
		: m_bWait(true)
		, m_nQueueBegin(0)
		, m_nQueueEnd(0)
		, m_nQueueCapacity(_DEFAULT_MESSAGE_QUEUE)
	{
		this->m_pMessageQueue = nullptr;
	}

	CMessageQueue::~CMessageQueue()
	{
		SAFE_DELETE_ARRAY(this->m_pMessageQueue);
	}

	bool CMessageQueue::init(bool bWait)
	{
		this->m_bWait = bWait;
		this->m_pMessageQueue = new SMessagePacket[this->m_nQueueCapacity];

		return true;
	}

	bool CMessageQueue::pushMessagePacket(const SMessagePacket& sMessagePacket)
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		this->m_pMessageQueue[this->m_nQueueEnd] = sMessagePacket;
		if (++this->m_nQueueEnd >= this->m_nQueueCapacity)
			this->m_nQueueEnd = 0;

		if (this->m_nQueueBegin == this->m_nQueueEnd)
		{
			SMessagePacket* pNewMessageQueue = new SMessagePacket[this->m_nQueueCapacity * 2];
			for (int32_t i = 0; i < this->m_nQueueCapacity; ++i)
				pNewMessageQueue[i] = this->m_pMessageQueue[(this->m_nQueueBegin + i) % this->m_nQueueCapacity];
			this->m_nQueueBegin = 0;
			this->m_nQueueEnd = this->m_nQueueCapacity;
			this->m_nQueueCapacity *= 2;
			SAFE_DELETE_ARRAY(this->m_pMessageQueue);
			this->m_pMessageQueue = pNewMessageQueue;
		}

		if (this->m_bWait && (this->m_nQueueEnd - this->m_nQueueBegin == 1 || this->m_nQueueBegin - this->m_nQueueEnd == this->m_nQueueCapacity - 1))
			this->m_cond.notify_one();

		return true;
	}

	void CMessageQueue::popMessagePacket(std::vector<SMessagePacket>& vecMessagePacket)
	{
		vecMessagePacket.clear();

		std::unique_lock<std::mutex> guard(this->m_lock);
		
		while (this->m_bWait && (this->m_nQueueEnd - this->m_nQueueBegin == 0))
		{
			this->m_cond.wait(guard);
		}
		
		while (this->m_nQueueEnd - this->m_nQueueBegin != 0)
		{
			const SMessagePacket& sMessagePacket = this->m_pMessageQueue[this->m_nQueueBegin];
			vecMessagePacket.push_back(sMessagePacket);

			if (++this->m_nQueueBegin >= this->m_nQueueCapacity)
				this->m_nQueueBegin = 0;
		}
	}
}