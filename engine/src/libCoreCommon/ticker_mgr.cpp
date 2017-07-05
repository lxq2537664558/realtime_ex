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
		
		std::unique_lock<base::spin_lock> lock(this->m_lock);
		this->insertTicker(pTickerNode);
	}

	void CTickerMgr::unregisterTicker(CTicker* pTicker)
	{
		DebugAst(pTicker != nullptr);

		std::unique_lock<base::spin_lock> lock(this->m_lock);
		if (!pTicker->isRegister())
			return;

		if (pTicker->m_pTickerNode->isLink())
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
		if ((pTickerNode->Value.pTicker->m_nNextTickTime | __TIME_NEAR_MASK) == (this->m_nLogicTime | __TIME_NEAR_MASK))
		{
			// 最近的定时器
			uint32_t nPos = (uint32_t)(pTickerNode->Value.pTicker->m_nNextTickTime&__TIME_NEAR_MASK);
			this->m_listNearTicker[nPos].pushTail(pTickerNode);
		}
		else
		{
			// 远程的定时器，先计算出处于哪一个联级
			uint32_t nLevel = 0;
			int64_t nMask = __TIME_NEAR_SIZE << __TIME_CASCADE_BITS;
			for (; nLevel < _countof(this->m_listCascadeTicker); ++nLevel)
			{
				if ((pTickerNode->Value.pTicker->m_nNextTickTime | (nMask - 1)) == (this->m_nLogicTime | (nMask - 1)))
					break;

				nMask <<= __TIME_CASCADE_BITS;
			}

			if (nLevel < _countof(this->m_listCascadeTicker))
			{
				// 在联级链表中
				uint32_t nPos = (uint32_t)((pTickerNode->Value.pTicker->m_nNextTickTime >> (__TIME_NEAR_BITS + nLevel * __TIME_CASCADE_BITS))&__TIME_CASCADE_MASK);
				DebugAst(nPos != 0);

				this->m_listCascadeTicker[nLevel][nPos].pushTail(pTickerNode);
			}
			else
			{
				// 巨大的超时时间，直接放到远端链表中
				this->m_listFarTicker.pushTail(pTickerNode);
			}
		}
	}

	void CTickerMgr::update(int64_t nTime)
	{
		// 每一次更新都将刻度时间慢慢推进到与当前时间一样
		// 处理时间定时器
		for (; this->m_nLogicTime < nTime; ++this->m_nLogicTime)
		{
			uint32_t nPos = (uint32_t)(this->m_nLogicTime&__TIME_NEAR_MASK);
			auto& listTicker = this->m_listNearTicker[nPos];
			this->m_vecTempTickerNode.clear();
			this->m_lock.lock();
			while (!listTicker.empty())
			{
				TickerNode_t* pTickerNode = listTicker.getHead();
				pTickerNode->remove();
				CTicker* pTicker = pTickerNode->Value.pTicker;

				if (pTicker->m_nIntervalTime == 0)
					this->unregisterTicker(pTicker);

				if (pTicker->m_callback != nullptr)
					pTicker->m_callback(pTicker->m_nContext);

				// 这个时候pTicker是不能动的，极有可能是个野指针
				if (pTickerNode->Value.pTicker == nullptr)
				{
					SAFE_DELETE(pTickerNode);
					continue;
				}
				pTicker->m_nNextTickTime += pTicker->m_nIntervalTime;
				this->m_vecTempTickerNode.push_back(pTickerNode);
			}
			size_t nCount = this->m_vecTempTickerNode.size();
			for (size_t i = 0; i < nCount; ++i)
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

			this->cascadeTicker();
			this->m_lock.lock();
		}
	}

	void CTickerMgr::cascadeTicker()
	{
		int64_t nCascadeTime = this->m_nLogicTime >> __TIME_NEAR_BITS;
		int64_t nMask = __TIME_NEAR_SIZE;
		// 一个联级一个联级的往上找，把符合条件的定时器重新添加
		for (uint32_t nLevel = 0; nLevel < _countof(this->m_listCascadeTicker); ++nLevel)
		{
			// 判断是否有进位发生
			if ((this->m_nLogicTime & (nMask - 1)) != 0)
				break;

			uint32_t nPos = nCascadeTime & __TIME_CASCADE_MASK;
			// 如果是0，肯定是要再计算上一个联级的
			if (nPos != 0)
			{
				auto& listTicker = this->m_listCascadeTicker[nLevel][nPos];
				while (!listTicker.empty())
				{
					TickerNode_t* pTickerNode = listTicker.getHead();
					pTickerNode->remove();
					
					this->insertTicker(pTickerNode);
				}
				break;
			}
			nMask <<= __TIME_CASCADE_BITS;
			nCascadeTime >>= __TIME_CASCADE_BITS;
			++nLevel;
		}

		// 击穿了前面所有联级，只能遍历所有的Far链表，这个对性能损耗非常大，不过基本不会发生
		if (nMask > (1LL << (__TIME_NEAR_BITS + __TIME_CASCADE_BITS * __TIME_CASCADE_COUNT)))
		{
			this->m_vecTempTickerNode.clear();
			while (!this->m_listFarTicker.empty())
			{
				TickerNode_t* pTickerNode = this->m_listFarTicker.getHead();
				pTickerNode->remove();
				
				this->m_vecTempTickerNode.push_back(pTickerNode);
			}

			size_t nCount = this->m_vecTempTickerNode.size();
			for (size_t i = 0; i < nCount; ++i)
			{
				TickerNode_t* pTickerNode = this->m_vecTempTickerNode[i];
				this->insertTicker(this->m_vecTempTickerNode[i]);
			}
		}
	}

}