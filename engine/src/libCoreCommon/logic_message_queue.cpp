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

		this->m_listMessagePacket.push_back(sMessagePacket);
		if (this->m_nFlag == eNotInList)
		{
			this->m_nFlag = eInList;
			this->m_pMessageQueueMgr->putMessageQueue(this);
		}
	}

	void CLogicMessageQueue::recv(std::list<SMessagePacket>& listMessagePacket)
	{
		PROFILING_GUARD(CLogicMessageQueue::recv);
		
		std::unique_lock<std::mutex> guard(this->m_lock);

		DebugAst(this->m_nFlag == eInList);

		listMessagePacket = std::move(this->m_listMessagePacket);

		this->m_nFlag = eDispatch;
	}

	void CLogicMessageQueue::dispatchEnd()
	{
		std::unique_lock<std::mutex> guard(this->m_lock);

		DebugAst(this->m_nFlag == eDispatch);

		if (!this->m_listMessagePacket.empty())
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