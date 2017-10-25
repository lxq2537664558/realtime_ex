#include "logic_message_queue_mgr.h"

#include "libBaseCommon/profiling.h"
#include "libBaseCommon/debug_helper.h"

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
		
		// 先notify再unlock的顺序没问题，并且不会带来等待线程醒来但是拿不到锁再次睡眠的问题，这个会有一个等待转移的过程，直接在内核处理掉了

		if (bNotify)
			this->m_cond.notify_one();	// 这里只需要唤醒一个等待线程就行如果存在等待线程，不然多个唤醒最后也会抢不到队列再次睡眠
	}
}