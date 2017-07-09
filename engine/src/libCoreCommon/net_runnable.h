#pragma once

#include "libBaseCommon/thread_base.h"
#include "net_message_queue.h"

namespace core
{
	class CCoreConnectionMgr;
	class CCoreApp;
	class CNetRunnable :
		public base::IRunnable
	{
		friend class CCoreApp;

	public:
		CNetRunnable();
		virtual ~CNetRunnable();

		static CNetRunnable*Inst();

		bool				init(uint32_t nMaxSocketCount);
		CCoreConnectionMgr*	getCoreConnectionMgr() const;
		CNetMessageQueue*	getMessageQueue() const;

		void				release();

	private:
		virtual bool		onInit();
		virtual bool		onProcess();
		virtual void		onDestroy();

	private:
		CCoreConnectionMgr*	m_pCoreConnectionMgr;
		base::CThreadBase*	m_pThreadBase;
		CNetMessageQueue*	m_pMessageQueue;
		int64_t				m_nLastCheckTime;
		int64_t				m_nTotalSamplingTime;
	};
}