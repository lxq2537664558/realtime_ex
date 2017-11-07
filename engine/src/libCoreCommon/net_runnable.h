#pragma once

#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/singleton.h"

#include "net_message_queue.h"

namespace core
{
	class CCoreConnectionMgr;
	class CCoreApp;
	class CNetRunnable :
		public base::IRunnable,
		public base::CSingleton<CNetRunnable>
	{
		friend class CCoreApp;

	public:
		CNetRunnable();
		virtual ~CNetRunnable();

		bool				init(uint32_t nMaxSocketCount);
		CCoreConnectionMgr*	getCoreConnectionMgr() const;
		CNetMessageQueue*	getMessageQueue() const;

	private:
		virtual bool		onInit();
		virtual bool		onProcess();
		virtual void		onDestroy();

	private:
		base::CThreadBase*			m_pThreadBase;
		CCoreConnectionMgr*			m_pCoreConnectionMgr;
		CNetMessageQueue*			m_pMessageQueue;
		std::vector<SMessagePacket>	m_vecMessagePacket;
		int64_t						m_nLastCheckTime;
		int64_t						m_nTotalSamplingTime;
	};
}