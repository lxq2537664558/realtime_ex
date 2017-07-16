#pragma once

#include "libBaseCommon/thread_base.h"

#include "logic_message_queue.h"
#include "base_connection_mgr.h"

namespace core
{
	class CLogicRunnable :
		public base::IRunnable
	{
	public:
		CLogicRunnable();
		virtual ~CLogicRunnable();

		static CLogicRunnable*Inst();

		bool				init();
		CLogicMessageQueue*	getMessageQueue() const;
		CBaseConnectionMgr*	getBaseConnectionMgr() const;
		void				sendInsideMessage(const SMessagePacket& sMessagePacket);
		void				recvInsideMessage(std::vector<SMessagePacket>& vecMessagePacket);
		
		void				release();

	private:
		virtual bool		onInit();
		virtual bool		onProcess();
		virtual void		onDestroy();

		bool				dispatch(const SMessagePacket& sMessagePacket);

	private:
		base::CThreadBase*							m_pThreadBase;
		CBaseConnectionMgr*							m_pBaseConnectionMgr;
		CLogicMessageQueue*							m_pMessageQueue;
		base::CCircleQueue<SMessagePacket, false>	m_insideQueue;
	};
}