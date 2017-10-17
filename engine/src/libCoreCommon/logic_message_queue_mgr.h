#pragma once
#include "logic_message_queue.h"

#include <list>

namespace core
{
	class CLogicMessageQueueMgr
	{
	public:
		CLogicMessageQueueMgr();
		~CLogicMessageQueueMgr();

		CLogicMessageQueue*	getMessageQueue();
		void				putMessageQueue(CLogicMessageQueue* pMessageQueue);

	private:
		std::list<CLogicMessageQueue*>	m_listMessageQueue;
		std::mutex						m_lock;
		std::condition_variable			m_cond;
	};
}