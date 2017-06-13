#pragma once

#include "ticker.h"

namespace core
{
	class CTickerMgr :
		public base::noncopyable
	{
	private:
		enum
		{
			__TIME_NEAR_BITS		= 16,
			__TIME_CASCADE_BITS		= 8,
			__TIME_CASCADE_COUNT	= 3,
			__TIME_NEAR_SIZE		= 1 << __TIME_NEAR_BITS,
			__TIME_NEAR_MASK		= __TIME_NEAR_SIZE - 1,
			__TIME_CASCADE_SIZE		= 1 << __TIME_CASCADE_BITS,
			__TIME_CASCADE_MASK		= __TIME_CASCADE_SIZE - 1,
			
		};

	public:
		CTickerMgr();
		~CTickerMgr();

		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void				unregisterTicker(CTicker* pTicker);
		void				update();
		inline int64_t		getLogicTime() const { return this->m_nLogicTime; }
		
	private:
		void				insertTicker(TickerNode_t* pTickerNode);
		void				cascadeTicker();

	private:
		base::TLink<TickerNode_t>	m_listNearTicker[__TIME_NEAR_SIZE];								// 最近运行到的时间刻度
		base::TLink<TickerNode_t>	m_listCascadeTicker[__TIME_CASCADE_COUNT][__TIME_CASCADE_SIZE];	// 联级时间刻度
		base::TLink<TickerNode_t>	m_listFarTicker;												// 最远的定时器链表

		std::vector<TickerNode_t*>	m_vecTempTickerNode;
		int64_t						m_nLogicTime;													// 当前刻度时间
	};
}