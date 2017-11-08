#include "stdafx.h"
#include "ticker_mgr_impl.h"

#include "debug_helper.h"
#include "time_util.h"
#include "profiling.h"

namespace base
{
	CTickerMgrImpl::CTickerMgrImpl(int64_t nTime, const std::function<void(CTicker*)>& callback)
		: m_nLogicTime(nTime)
		, m_callback(callback)
	{
	}

	CTickerMgrImpl::~CTickerMgrImpl()
	{
	}

	int64_t CTickerMgrImpl::getLogicTime() const
	{
		return this->m_nLogicTime;
	}

	bool CTickerMgrImpl::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		DebugAstEx(pTicker != nullptr, false);
		DebugAstEx(!pTicker->isRegister(), false);
		DebugAstEx(pTicker->getCallback() != nullptr, false);
		
		CCoreTickerNode* pCoreTickerNode = new CCoreTickerNode();
		pCoreTickerNode->Value.pTickerMgr = this;
		pCoreTickerNode->Value.pTicker = pTicker;
		pCoreTickerNode->Value.nNextTime = this->m_nLogicTime + nStartTime;
		pCoreTickerNode->Value.nIntervalTime = nIntervalTime;
		
		pTicker->m_nIntervalTime = nIntervalTime;
		pTicker->m_nContext = nContext;
		pTicker->m_pCoreContext = pCoreTickerNode;

		this->insertTicker(pCoreTickerNode);

		return true;
	}

	void CTickerMgrImpl::unregisterTicker(CTicker* pTicker)
	{
		DebugAst(pTicker != nullptr);

		CCoreTickerNode* pCoreTickerNode = reinterpret_cast<CCoreTickerNode*>(pTicker->m_pCoreContext);
		if (nullptr == pCoreTickerNode)
			return;

		if (pCoreTickerNode->isLink())
		{
			// 正常的反注册分支
			pCoreTickerNode->remove();
			pCoreTickerNode->Value.pTicker = nullptr;
			pCoreTickerNode->Value.pTickerMgr = nullptr;
			pTicker->m_pCoreContext = nullptr;
			SAFE_DELETE(pCoreTickerNode);
		}
		else
		{
			// 定时器回调的时候反注册分支
			pCoreTickerNode->Value.pTicker = nullptr;
			pCoreTickerNode->Value.pTickerMgr = nullptr;
			pTicker->m_pCoreContext = nullptr;
		}
	}

	void CTickerMgrImpl::insertTicker(CCoreTickerNode* pTickerNode)
	{
		DebugAst(pTickerNode->Value.nNextTime >= this->m_nLogicTime);

		int64_t nDeltaTime = pTickerNode->Value.nNextTime - this->m_nLogicTime;

		if (nDeltaTime < __TIME_NEAR_SIZE)
		{
			// 最近的定时器
			uint32_t nPos = (uint32_t)(pTickerNode->Value.nNextTime&__TIME_NEAR_MASK);
			
			this->m_listNearTicker[nPos].pushTail(pTickerNode);
		}
		else
		{
			// 远程的定时器，先计算出处于哪一个联级
			uint32_t nLevel = 0;
			int64_t nMask = __TIME_NEAR_SIZE << __TIME_CASCADE_BITS;
			for (; nLevel < _countof(this->m_listCascadeTicker); ++nLevel)
			{
				if (nDeltaTime < nMask)
					break;

				nMask <<= __TIME_CASCADE_BITS;
			}

			if (nLevel < _countof(this->m_listCascadeTicker))
			{
				// 在联级链表中
				uint32_t nPos = (uint32_t)((pTickerNode->Value.nNextTime >> (__TIME_NEAR_BITS + nLevel * __TIME_CASCADE_BITS))&__TIME_CASCADE_MASK);
				
				this->m_listCascadeTicker[nLevel][nPos].pushTail(pTickerNode);
			}
			else
			{
				// 巨大的超时时间，直接放到远端链表中
				this->m_listFarTicker.pushTail(pTickerNode);
			}
		}
	}

	void CTickerMgrImpl::update(int64_t nTime)
	{
		// 每一次更新都将刻度时间慢慢推进到与当前时间一样
		// 处理时间定时器
		for (; this->m_nLogicTime <= nTime; ++this->m_nLogicTime)
		{
			uint32_t nPos = (uint32_t)(this->m_nLogicTime&__TIME_NEAR_MASK);
			if (nPos == 0)
			{
				// 进位了，做联级计算
				this->cascadeTicker();
			}

			auto& listTicker = this->m_listNearTicker[nPos];
			this->m_vecTempTickerNode.clear();

			while (!listTicker.empty())
			{
				CCoreTickerNode* pCoreTickerNode = listTicker.getHead();
				pCoreTickerNode->remove();

				CTicker* pTicker = pCoreTickerNode->Value.pTicker;

				// 对于一次性的定时器必须在回调前反注册掉
				if (pTicker->m_nIntervalTime == 0)
				{
					pCoreTickerNode->Value.pTicker = nullptr;
					pTicker->m_pCoreContext = nullptr;
				}

				auto& callback = pTicker->getCallback();
				if (callback != nullptr)
				{
					if (this->m_callback != nullptr)
					{
						this->m_callback(pTicker);
					}
					else
					{
						callback(pTicker->getContext());
					}
				}

				// 回调的时候反注册了或者一次性定时器，这个时候pTicker是不能动的，极有可能是个野指针
				if (pCoreTickerNode->Value.pTicker == nullptr)
				{
					SAFE_DELETE(pCoreTickerNode);
					continue;
				}
				
				pCoreTickerNode->Value.nNextTime += pCoreTickerNode->Value.nIntervalTime;
				this->m_vecTempTickerNode.push_back(pCoreTickerNode);
			}

			size_t nCount = this->m_vecTempTickerNode.size();
			for (size_t i = 0; i < nCount; ++i)
			{
				CCoreTickerNode* pCoreTickerNode = this->m_vecTempTickerNode[i];
				// 这段时间如果该定时器被反注册了，需要删除掉pCoreTickerNode
				if (pCoreTickerNode->Value.pTicker == nullptr)
				{
					SAFE_DELETE(pCoreTickerNode);
					continue;
				}

				this->insertTicker(pCoreTickerNode);
			}
		}
	}

	void CTickerMgrImpl::cascadeTicker()
	{
		uint32_t nLevel = 0;
		// 一个联级一个联级的往上找，把符合条件的定时器重新添加
		for (; nLevel < _countof(this->m_listCascadeTicker); ++nLevel)
		{
			int64_t nCascadeTime = (this->m_nLogicTime >> (__TIME_NEAR_BITS + nLevel * __TIME_CASCADE_BITS));

			uint32_t nPos = nCascadeTime & __TIME_CASCADE_MASK;
			auto& listTicker = this->m_listCascadeTicker[nLevel][nPos];
			while (!listTicker.empty())
			{
				CCoreTickerNode* pTickerNode = listTicker.getHead();
				pTickerNode->remove();

				this->insertTicker(pTickerNode);
			}
			// 如果不是0，停止再计算上一个联级的
			if (nPos != 0)
				break;
		}

		// 击穿了前面所有联级，只能遍历所有的Far链表，这个对性能损耗非常大，不过基本不会发生
		if (nLevel >= _countof(this->m_listCascadeTicker))
		{
			while (!this->m_listFarTicker.empty())
			{
				CCoreTickerNode* pCoreTickerNode = this->m_listFarTicker.getHead();
				pCoreTickerNode->remove();

				this->insertTicker(pCoreTickerNode);
			}
		}
	}
}