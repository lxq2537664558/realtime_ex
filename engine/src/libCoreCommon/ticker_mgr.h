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
			__TIME_WHEEL_SIZE		= 0x00010000,	///< 刻度尺的长度 大部分情况下这个长度应该足够了，很少会有定时长度超过2^16（65536）ms的，即使存在也没事
			__TIME_WHEEL_MASK		= 0x0000ffff,	///< 定时时间掩码
			__TIME_WHEEL_PRECISION	= 10,			///< 精度10ms
		};

	private:
		base::CTinyList<TickerNode_t>	m_listTicker[__TIME_WHEEL_SIZE];		///< 各个定时器挂在对应刻度的列表上， 可能两个触发时间相差2^16ms的定时器会挂在想通链表上，需要做处理
		std::vector<TickerNode_t*>		m_vecTempTickerNode;
		int64_t							m_nLogicTime;							///< 当前刻度时间

	public:
		CTickerMgr();
		~CTickerMgr();

		void				insertTicker(TickerNode_t* pTickerNode);
		void				registTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void				unregistTicker(CTicker* pTicker);
		void				update();
		inline uint64_t		getLogicTime() const { return this->m_nLogicTime; }
		uint32_t			getNearestTime() const;
	};
}