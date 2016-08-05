#include "stdafx.h"
#include "logic_message_queue.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/profiling.h"

#define _DEFAULT_MESSAGE_QUEUE 1024

namespace core
{
	CLogicMessageQueue::CLogicMessageQueue()
		: m_queue(_DEFAULT_MESSAGE_QUEUE)
	{
	}

	CLogicMessageQueue::~CLogicMessageQueue()
	{
	}

	bool CLogicMessageQueue::init()
	{
		return true;
	}

	void CLogicMessageQueue::send(const SMessagePacket& sMessagePacket)
	{
		PROFILING_GUARD(CLogicMessageQueue::send);
		std::unique_lock<std::mutex> guard(this->m_lock);

		this->m_queue.send(sMessagePacket);

		if (this->m_queue.size() == 1)
			this->m_cond.notify_one();
	}

	void CLogicMessageQueue::recv(std::vector<SMessagePacket>& vecMessagePacket, bool bWait)
	{
		PROFILING_GUARD(CLogicMessageQueue::recv);
		vecMessagePacket.clear();

		std::unique_lock<std::mutex> guard(this->m_lock);
		
		while (bWait && this->m_queue.empty())
		{
			this->m_cond.wait(guard);
		}
		
		SMessagePacket sMessagePacket;
		while (this->m_queue.recv(sMessagePacket))
		{
			vecMessagePacket.push_back(sMessagePacket);
		}
	}
}