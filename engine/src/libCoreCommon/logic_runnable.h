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
		
		void				release();

	private:
		virtual bool		onInit();
		virtual bool		onProcess();
		virtual void		onDestroy();

	private:
		base::CThreadBase*	m_pThreadBase;
		CBaseConnectionMgr*	m_pBaseConnectionMgr;
	};
}