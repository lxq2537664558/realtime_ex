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
		int64_t			nNextTime;		// ��һ�ζ�ʱ������ʱ��
		int64_t			nIntervalTime;	// ��ʱ�����еļ��ʱ��
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
		base::TLink<CCoreTickerNode>	m_listNearTicker[__TIME_NEAR_SIZE];								// ������е���ʱ��̶�
		base::TLink<CCoreTickerNode>	m_listCascadeTicker[__TIME_CASCADE_COUNT][__TIME_CASCADE_SIZE];	// ����ʱ��̶�
		base::TLink<CCoreTickerNode>	m_listFarTicker;												// ��Զ�Ķ�ʱ������

		std::vector<CCoreTickerNode*>	m_vecTempTickerNode;
		int64_t							m_nLogicTime;													// ��ǰ�̶�ʱ��

		std::function<void(CTicker*)>	m_callback;
	};
}