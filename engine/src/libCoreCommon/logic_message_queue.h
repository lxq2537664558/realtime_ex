#pragma once
#include <condition_variable>

#include "message_queue.h"

namespace core
{
	class CLogicMessageQueueMgr;
	class CCoreService;
	class CLogicMessageQueue :
		public CMessageQueue
	{
	public:
		CLogicMessageQueue(CCoreService* pCoreService, CLogicMessageQueueMgr* pMessageQueueMgr);
		virtual ~CLogicMessageQueue();

		CCoreService*	getCoreService() const;

		void			send(const SMessagePacket& sMessagePacket);
		void			recv(std::vector<SMessagePacket>& vecMessagePacket);
		void			dispatchEnd();

	private:
		enum
		{
			eInList,
			eNotInList,
			eDispatch,
		};

		CLogicMessageQueueMgr*	m_pMessageQueueMgr;
		CCoreService*			m_pCoreService;
		uint32_t				m_nFlag;
	};
}