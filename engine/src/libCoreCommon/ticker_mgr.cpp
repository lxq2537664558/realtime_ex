#include "stdafx.h"
#include "ticker_mgr.h"
#include "monitor.h"

#include "libBaseCommon/base_time.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/profiling.h"

namespace core
{
	CTickerMgr::CTickerMgr()
	{
		this->m_nLogicTime = base::getGmtTime();
	}

	CTickerMgr::~CTickerMgr()
	{

	}

	uint32_t CTickerMgr::getNearestTime() const
	{
		int64_t nCurTime = base::getGmtTime();
		int64_t nDeltaTime = this->m_nLogicTime + __TIME_WHEEL_PRECISION - nCurTime;
		if (nDeltaTime < 0)
			nDeltaTime = 0;

		return (uint32_t)nDeltaTime;
	}

	void CTickerMgr::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		DebugAst(pTicker != nullptr);
		DebugAst(!pTicker->isRegister());
		DebugAst(pTicker->m_callback != nullptr);

		TickerNode_t* pTickerNode = new TickerNode_t();
		pTickerNode->Value.pTicker = pTicker;
		pTicker->m_nIntervalTime = nIntervalTime;
		pTicker->m_nNextTickTime = this->m_nLogicTime + nStartTime;
		pTicker->m_nContext = nContext;
		pTicker->m_pTickerNode = pTickerNode;

		this->insertTicker(pTickerNode);
	}

	void CTickerMgr::unregisterTicker(CTicker* pTicker)
	{
		DebugAst(pTicker != nullptr);
		if (!pTicker->isRegister())
			return;

		if (pTicker->m_pTickerNode->isHang())
		{
			pTicker->m_pTickerNode->remove();
			pTicker->m_pTickerNode->Value.pTicker = nullptr;
			SAFE_DELETE(pTicker->m_pTickerNode);
		}
		else
		{
			pTicker->m_pTickerNode->Value.pTicker = nullptr;
			pTicker->m_pTickerNode = nullptr;
		}
	}

	void CTickerMgr::insertTicker(TickerNode_t* pTickerNode)
	{
		DebugAst(pTickerNode != nullptr && pTickerNode->Value.pTicker != nullptr);
		uint32_t nPos = (uint32_t)(pTickerNode->Value.pTicker->m_nNextTickTime&__TIME_WHEEL_MASK) / __TIME_WHEEL_PRECISION;
		auto& listTicker = this->m_listTicker[nPos];
		listTicker.pushBack(pTickerNode);
	}

	void CTickerMgr::update()
	{
		int64_t nCurTime = base::getGmtTime();
		// 每一次更新都将刻度时间慢慢推进到与当前时间一样
		// 处理时间定时器
		for (; this->m_nLogicTime < nCurTime; this->m_nLogicTime += __TIME_WHEEL_PRECISION)
		{
			uint32_t nPos = (uint32_t)(this->m_nLogicTime&__TIME_WHEEL_MASK) / __TIME_WHEEL_PRECISION;
			auto& listTicker = this->m_listTicker[nPos];
			this->m_vecTempTickerNode.clear();
			while (!listTicker.isEmpty())
			{
				TickerNode_t* pTickerNode = listTicker.getFront();
				pTickerNode->remove();
				CTicker* pTicker = pTickerNode->Value.pTicker;
				if (pTicker->m_nNextTickTime <= this->m_nLogicTime)
				{
#ifdef __PROFILING_OPEN
					
#endif
					if (pTicker->m_nIntervalTime == 0)
						this->unregisterTicker(pTicker);

					if (pTicker->m_callback != nullptr)
						pTicker->m_callback(pTicker->m_nContext);

#ifdef __PROFILING_OPEN
#endif
					// 这个时候pTicker是不能动的，极有可能是个野指针
					if (pTickerNode->Value.pTicker == nullptr)
					{
						SAFE_DELETE(pTickerNode);
						continue;
					}
					pTicker->m_nNextTickTime += pTicker->m_nIntervalTime;
					this->m_vecTempTickerNode.push_back(pTickerNode);
				}
				else
				{
					this->m_vecTempTickerNode.push_back(pTickerNode);
				}
			}
			uint32_t nCount = (uint32_t)this->m_vecTempTickerNode.size();
			for (uint32_t i = 0; i < nCount; ++i)
			{
				TickerNode_t* pTickerNode = this->m_vecTempTickerNode[i];
				// 有可能后执行的定时器删除了前执行的定时器
				if (pTickerNode->Value.pTicker == nullptr)
				{
					SAFE_DELETE(pTickerNode);
					continue;
				}
				this->insertTicker(this->m_vecTempTickerNode[i]);
			}
		}
	}
}