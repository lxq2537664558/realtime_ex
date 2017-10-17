#include "logic_message_queue_mgr.h"

#include "libBaseCommon/profiling.h"

namespace core
{

	CLogicMessageQueueMgr::CLogicMessageQueueMgr()
	{

	}

	CLogicMessageQueueMgr::~CLogicMessageQueueMgr()
	{

	}

	CLogicMessageQueue* CLogicMessageQueueMgr::getMessageQueue()
	{
		PROFILING_GUARD(CLogicMessageQueueMgr::getMessageQueue);
		std::unique_lock<std::mutex> guard(this->m_lock);

		while (this->m_listMessageQueue.empty())
		{
			this->m_cond.wait(guard);
		}
		
		CLogicMessageQueue* pMessageQueue = this->m_listMessageQueue.front();
		this->m_listMessageQueue.pop_front();
		
		return pMessageQueue;
	}

	void CLogicMessageQueueMgr::putMessageQueue(CLogicMessageQueue* pMessageQueue)
	{
		DebugAst(pMessageQueue != nullptr);

		std::unique_lock<std::mutex> guard(this->m_lock);

		bool bNotify = this->m_listMessageQueue.empty();
		this->m_listMessageQueue.push_back(pMessageQueue);
		
		if (bNotify)
			this->m_cond.notify_one();
	}
}