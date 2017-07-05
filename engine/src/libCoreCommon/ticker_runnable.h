#pragma once

#include "libBaseCommon/thread_base.h"
#include "ticker_mgr.h"

namespace core
{
	class CTickerRunnable :
		public base::IRunnable
	{
		friend class CCoreApp;

	public:
		CTickerRunnable();
		virtual ~CTickerRunnable();

		static CTickerRunnable*Inst();

		bool				init();

		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void				unregisterTicker(CTicker* pTicker);
		
		void				join();
		void				release();

	private:
		virtual bool		onInit();
		virtual bool		onProcess();
		virtual void		onDestroy();

	private:
		base::CThreadBase*	m_pThreadBase;
		CTickerMgr*			m_pTickerMgr;
	};
}