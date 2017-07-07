#include "stdafx.h"
#include "ticker_runnable.h"
#include "core_common.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>
#include <thread>

// 放这里为了调试或者看dump的时候方便
core::CTickerRunnable*	g_pTickerRunnable;

#define _CYCLE_TIME 10


namespace core
{
	CTickerRunnable::CTickerRunnable()
		: m_pThreadBase(nullptr)
	{
	}

	CTickerRunnable::~CTickerRunnable()
	{
		SAFE_RELEASE(this->m_pThreadBase);
	}

	CTickerRunnable* CTickerRunnable::Inst()
	{
		if (g_pTickerRunnable == nullptr)
			g_pTickerRunnable = new core::CTickerRunnable();

		return g_pTickerRunnable;
	}

	bool CTickerRunnable::init()
	{
		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
	}

	void CTickerRunnable::join()
	{
		this->m_pThreadBase->join();
	}

	void CTickerRunnable::release()
	{
		delete g_pTickerRunnable;
		g_pTickerRunnable = nullptr;
	}

	bool CTickerRunnable::onInit()
	{
		return true;
	}

	void CTickerRunnable::onDestroy()
	{

	}

	bool CTickerRunnable::onProcess()
	{
		int64_t nCurTime = base::getGmtTime();
		this->update(nCurTime);
		int64_t nEndTime = base::getGmtTime();
		int64_t nDeltaTime = nEndTime - nCurTime;
		if (_CYCLE_TIME - nDeltaTime > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(_CYCLE_TIME - nDeltaTime));

		return true;
	}

	bool CTickerRunnable::registerTicker(uint64_t nFrom, uint32_t nType, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		DebugAstEx(pTicker != nullptr, false);
		DebugAstEx(!pTicker->isRegister(), false);

		CCoreTickerNode* pCoreTickerNode = new CCoreTickerNode();
		pCoreTickerNode->Value.m_pTicker = pTicker;
		pCoreTickerNode->Value.m_nNextTime = this->m_nLogicTime + nStartTime;
		pCoreTickerNode->Value.m_pMemory = pCoreTickerNode;
		
		pTicker->m_nIntervalTime = nIntervalTime;
		pTicker->m_nContext = nContext;
		pTicker->m_pCoreContext = pCoreTickerNode;

		std::unique_lock<base::spin_lock> lock(this->m_lock);
		this->insertTicker(pCoreTickerNode);
	}

	void CTickerRunnable::unregisterTicker(CTicker* pTicker)
	{
		DebugAst(pTicker != nullptr);

		if (!pTicker->isRegister())
			return;

		std::unique_lock<base::spin_lock> lock(this->m_lock);

		CCoreTickerNode* pCoreTickerNode = reinterpret_cast<CCoreTickerNode*>(pTicker->m_pCoreContext);
		if (pCoreTickerNode->isLink())
			pCoreTickerNode->remove();

		pCoreTickerNode->Value.m_pTicker = nullptr;
		pTicker->m_pCoreContext = nullptr;
		pCoreTickerNode->Value.release();
	}

	void CTickerRunnable::insertTicker(CCoreTickerNode* pTickerNode)
	{
		if ((pTickerNode->Value.m_nNextTime | __TIME_NEAR_MASK) == (this->m_nLogicTime | __TIME_NEAR_MASK))
		{
			// 最近的定时器
			uint32_t nPos = (uint32_t)(pTickerNode->Value.m_nNextTime&__TIME_NEAR_MASK);
			this->m_listNearTicker[nPos].pushTail(pTickerNode);
		}
		else
		{
			// 远程的定时器，先计算出处于哪一个联级
			uint32_t nLevel = 0;
			int64_t nMask = __TIME_NEAR_SIZE << __TIME_CASCADE_BITS;
			for (; nLevel < _countof(this->m_listCascadeTicker); ++nLevel)
			{
				if ((pTickerNode->Value.m_nNextTime | (nMask - 1)) == (this->m_nLogicTime | (nMask - 1)))
					break;

				nMask <<= __TIME_CASCADE_BITS;
			}

			if (nLevel < _countof(this->m_listCascadeTicker))
			{
				// 在联级链表中
				uint32_t nPos = (uint32_t)((pTickerNode->Value.m_nNextTime >> (__TIME_NEAR_BITS + nLevel * __TIME_CASCADE_BITS))&__TIME_CASCADE_MASK);
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

	void CTickerRunnable::update(int64_t nTime)
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
				CCoreTickerNode* pCoreTickerNode = listTicker.getHead();
				pCoreTickerNode->remove();
				
				this->onTicker(pCoreTickerNode);

				if (pCoreTickerNode->Value.m_pTicker->m_nIntervalTime == 0)
					continue;
				
				pCoreTickerNode->Value.m_nNextTime += pCoreTickerNode->Value.m_pTicker->m_nIntervalTime;
				this->m_vecTempTickerNode.push_back(pCoreTickerNode);
			}
			size_t nCount = this->m_vecTempTickerNode.size();
			for (size_t i = 0; i < nCount; ++i)
			{
				this->insertTicker(this->m_vecTempTickerNode[i]);
			}

			this->cascadeTicker();
			this->m_lock.lock();
		}
	}

	void CTickerRunnable::cascadeTicker()
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
					CCoreTickerNode* pTickerNode = listTicker.getHead();
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
				CCoreTickerNode* pTickerNode = this->m_listFarTicker.getHead();
				pTickerNode->remove();

				this->m_vecTempTickerNode.push_back(pTickerNode);
			}

			size_t nCount = this->m_vecTempTickerNode.size();
			for (size_t i = 0; i < nCount; ++i)
			{
				CCoreTickerNode* pTickerNode = this->m_vecTempTickerNode[i];
				this->insertTicker(this->m_vecTempTickerNode[i]);
			}
		}
	}

	void CTickerRunnable::onTicker(CCoreTickerNode* pCoreTickerNode)
	{
		DebugAst(pCoreTickerNode != nullptr);

		pCoreTickerNode->Value.addRef();
		// 发送到消息队列，另外消息队列取出定时器对象的时候如果发现是一次性定时器，需要反注册
	}

	CCoreTickerInfo::CCoreTickerInfo()
	{
		this->m_pTicker = nullptr;
		this->m_pMemory = nullptr;
		this->m_nNextTime = 0;
		this->m_nRef = 1;
	}

	void CCoreTickerInfo::addRef()
	{
		++this->m_nRef;
	}

	void CCoreTickerInfo::release()
	{
		if ((--this->m_nRef) == 0)
		{
			delete reinterpret_cast<CCoreTickerNode*>(this->m_pMemory);
		}
	}

	int32_t CCoreTickerInfo::getRef() const
	{
		return this->m_nRef.load();
	}
}