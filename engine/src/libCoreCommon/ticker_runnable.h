#pragma once

#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/link.h"

#include "ticker.h"

#include <memory>

namespace core
{
	class CTickerRunnable;
	class CCoreTickerInfo
	{
	public:
		enum
		{
			eRegister,
			eUnRegister,
			eDown,
		};

		friend class CTickerRunnable;

	public:
		CCoreTickerInfo();
		~CCoreTickerInfo();

		void	addRef();
		void	release();
		int32_t	getRef() const;

	private:
		CTicker*				pTicker;
		int64_t					nNextTime;	// 下一次定时器运行时间
		std::atomic<int32_t>	nRef;
		std::atomic<int32_t>	nState;
	};

	typedef base::TLinkNode<CCoreTickerInfo> TickerNode_t;

	class CTickerRunnable :
		public base::IRunnable
	{
	private:
		enum
		{
			__TIME_NEAR_BITS = 16,
			__TIME_CASCADE_BITS = 8,
			__TIME_CASCADE_COUNT = 3,
			__TIME_NEAR_SIZE = 1 << __TIME_NEAR_BITS,
			__TIME_NEAR_MASK = __TIME_NEAR_SIZE - 1,
			__TIME_CASCADE_SIZE = 1 << __TIME_CASCADE_BITS,
			__TIME_CASCADE_MASK = __TIME_CASCADE_SIZE - 1,
		};

	public:
		CTickerRunnable();
		virtual ~CTickerRunnable();

		static CTickerRunnable*	Inst();

		bool			init();

		bool			registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void			unregisterTicker(CTicker* pTicker);
		
		void			join();
		void			release();

	private:
		virtual bool	onInit();
		virtual bool	onProcess();
		virtual void	onDestroy();

		void			update(int64_t nTime);

		void			insertTicker(TickerNode_t* pTickerNode);
		void			cascadeTicker();
		void			onTicker(CTicker* pTicker);

	private:
		base::CThreadBase*			m_pThreadBase;
		base::TLink<TickerNode_t>	m_listNearTicker[__TIME_NEAR_SIZE];								// 最近运行到的时间刻度
		base::TLink<TickerNode_t>	m_listCascadeTicker[__TIME_CASCADE_COUNT][__TIME_CASCADE_SIZE];	// 联级时间刻度
		base::TLink<TickerNode_t>	m_listFarTicker;												// 最远的定时器链表

		std::vector<TickerNode_t*>	m_vecTempTickerNode;
		int64_t						m_nLogicTime;													// 当前刻度时间
		base::spin_lock				m_lock;
	};
}