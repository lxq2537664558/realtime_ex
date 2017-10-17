#include "stdafx.h"
#include "logic_message_queue.h"
#include "logic_message_queue_mgr.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/profiling.h"

namespace core
{
	CLogicMessageQueue::CLogicMessageQueue(CCoreService* pCoreService, CLogicMessageQueueMgr* pMessageQueueMgr)
		: m_pMessageQueueMgr(pMessageQueueMgr)
		, m_pCoreService(pCoreService)
		, m_nFlag(eNotInList)
	{
	}

	CLogicMessageQueue::~CLogicMessageQueue()
	{
	}

	CCoreService* CLogicMessageQueue::getCoreService() const
	{
		return this->m_pCoreService;
	}

	void CLogicMessageQueue::send(const SMessagePacket& sMessagePacket)
	{
		PROFILING_GUARD(CLogicMessageQueue::send);
		std::unique_lock<std::mutex> guard(this->m_lock);

		this->m_queue.send(sMessagePacket);
		if (this->m_nFlag == eNotInList)
		{
			this->m_nFlag = eInList;
			this->m_pMessageQueueMgr->putMessageQueue(this);
		}
	}

	void CLogicMessageQueue::recv(std::vector<SMessagePacket>& vecMessagePacket)
	{
		PROFILING_GUARD(CLogicMessageQueue::recv);
		vecMessagePacket.clear();

		std::unique_lock<std::mutex> guard(this->m_lock);

		DebugAst(this->m_nFlag == eInList);

		SMessagePacket sMessagePacket;
		while (this->m_queue.recv(sMessagePacket))
		{
			vecMessagePacket.push_back(sMessagePacket);
		}

		this->m_nFlag = eDispatch;
	}

	void CLogicMessageQueue::dispatchEnd()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		DebugAst(this->m_nFlag == eDispatch);

		if (this->m_queue.size() != 0)
		{
			this->m_nFlag = eInList;
			this->m_pMessageQueueMgr->putMessageQueue(this);
		}
		else
		{
			this->m_nFlag = eNotInList;
		}
	}
}