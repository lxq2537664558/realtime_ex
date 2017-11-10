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

		this->m_listMessageQueue.push_back(pMessageQueue);
		
		// 先notify再unlock的顺序没问题，并且不会带来等待线程醒来但是拿不到锁再次睡眠的问题，这个会有一个等待转移的过程，直接把线程等待到锁等待列表中
		// 如果先unlock再notify，会带来比较多的伪唤醒问题，当然逻辑正确性也是没问题的

		// 这里不在需要额外的控制变量来限制notify次数，因为如果没有线程在等待，这个函数啥也不会做，这个跟我们自己加标识一样
		this->m_cond.notify_one();
	}
}