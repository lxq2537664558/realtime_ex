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
	}

	void CLogicMessageQueue::recv(std::vector<SMessagePacket>& vecMessagePacket)
	{
		PROFILING_GUARD(CLogicMessageQueue::recv);
		vecMessagePacket.clear();

		std::unique_lock<std::mutex> guard(this->m_lock);
		
		SMessagePacket sMessagePacket;
		while (this->m_queue.recv(sMessagePacket))
		{
			vecMessagePacket.push_back(sMessagePacket);
		}
	}

	bool CLogicMessageQueue::empty()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		return this->m_queue.size();
	}
}