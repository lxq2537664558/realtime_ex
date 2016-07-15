#pragma once

#include "libBaseCommon/thread_base.h"
#include "message_queue.h"

namespace core
{
	class CCoreConnectionMgr;
	class CBaseAppImpl;
	class CNetRunnable :
		public base::IRunnable
	{
		friend class CBaseAppImpl;

	public:
		CNetRunnable();
		virtual ~CNetRunnable();

		static CNetRunnable*Inst();

		bool				init(uint32_t nMaxSocketCount);
		CCoreConnectionMgr*	getCoreConnectionMgr() const;
		CMessageQueue*		getMessageQueue() const;

		void				join();
		void				release();

	private:
		virtual bool		onInit();
		virtual bool		onProcess();
		virtual void		onDestroy();

	private:
		CCoreConnectionMgr*	m_pCoreConnectionMgr;
		base::CThreadBase*	m_pThreadBase;
		CMessageQueue*		m_pMessageQueue;
		int64_t				m_nLastCheckTime;
		int64_t				m_nTotalSamplingTime;
	};
}