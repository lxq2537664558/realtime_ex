#pragma once

#include "link.h"
#include "singleton.h"
#include "ticker.h"

#include <memory>
#include <vector>


namespace base
{
	class CTickerMgrImpl;
	class CTicker;
	struct SCoreTickerInfo
	{
		CTickerMgrImpl*	pTickerMgr;
		CTicker*		pTicker;
		int64_t			nNextTime;		// 下一次定时器运行时间
		int64_t			nIntervalTime;	// 定时器运行的间隔时间
	};

	typedef TLinkNode<SCoreTickerInfo> CCoreTickerNode;

	class CTickerMgrImpl
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
		CTickerMgrImpl(int64_t nTime, const std::function<void(CTicker*)>& callback);
		virtual ~CTickerMgrImpl();

		int64_t	getLogicTime() const;
		void	update(int64_t nTime);
		bool	registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		void	unregisterTicker(CTicker* pTicker);
		
	private:
		void	insertTicker(CCoreTickerNode* pCoreTickerNode);
		void	cascadeTicker();
		
	private:
		base::TLink<CCoreTickerNode>	m_listNearTicker[__TIME_NEAR_SIZE];								// 最近运行到的时间刻度
		base::TLink<CCoreTickerNode>	m_listCascadeTicker[__TIME_CASCADE_COUNT][__TIME_CASCADE_SIZE];	// 联级时间刻度
		base::TLink<CCoreTickerNode>	m_listFarTicker;												// 最远的定时器链表

		std::vector<CCoreTickerNode*>	m_vecTempTickerNode;
		int64_t							m_nLogicTime;													// 当前刻度时间

		std::function<void(CTicker*)>	m_callback;
	};
}