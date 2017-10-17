#include "stdafx.h"
#include "ticker_runnable.h"
#include "core_common.h"
#include "core_app.h"
#include "message_command.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/profiling.h"

#include <algorithm>
#include <thread>

#define _CYCLE_TIME 10

namespace core
{
	CTickerRunnable::CTickerRunnable()
		: m_pThreadBase(nullptr)
	{
		this->m_nLogicTime = base::time_util::getGmtTime();
	}

	CTickerRunnable::~CTickerRunnable()
	{
		SAFE_RELEASE(this->m_pThreadBase);
	}

	bool CTickerRunnable::init()
	{
		this->m_pThreadBase = base::CThreadBase::createNew(this);
		return nullptr != this->m_pThreadBase;
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
		int64_t nCurTime = base::time_util::getGmtTime();
		this->update(nCurTime);

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_FRAME;
		sMessagePacket.pData = nullptr;
		sMessagePacket.nDataSize = 0;

		const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService();
		for (size_t i = 0; i < vecCoreService.size(); ++i)
		{
			vecCoreService[i]->getMessageQueue()->send(sMessagePacket);
		}

		int64_t nEndTime = base::time_util::getGmtTime();
		int64_t nDeltaTime = nEndTime - nCurTime;
		if (_CYCLE_TIME - nDeltaTime > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(_CYCLE_TIME - nDeltaTime));

		return true;
	}

	bool CTickerRunnable::registerTicker(CMessageQueue* pMessageQueue, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		DebugAstEx(pTicker != nullptr, false);
		DebugAstEx(!pTicker->isRegister(), false);
		DebugAstEx(pTicker->getCallback() != nullptr, false);
		DebugAstEx(pMessageQueue != nullptr, false);

		CCoreTickerNode* pCoreTickerNode = new CCoreTickerNode();
		pCoreTickerNode->Value.m_pTicker = pTicker;
		pCoreTickerNode->Value.m_nNextTime = this->m_nLogicTime + nStartTime;
		pCoreTickerNode->Value.m_nIntervalTime = nIntervalTime;
		pCoreTickerNode->Value.m_pMemory = pCoreTickerNode;
		pCoreTickerNode->Value.m_pMessageQueue = pMessageQueue;
		pCoreTickerNode->Value.m_nState = eRegister;
		pCoreTickerNode->Value.m_nRef = 1;

		pTicker->m_nIntervalTime = nIntervalTime;
		pTicker->m_nContext = nContext;
		pTicker->m_pCoreContext = pCoreTickerNode;

		std::unique_lock<base::spin_lock> lock(this->m_lockRegister);
		this->m_vecRegisterTicker.push_back(pCoreTickerNode);

		return true;
	}

	void CTickerRunnable::unregisterTicker(CTicker* pTicker)
	{
		DebugAst(pTicker != nullptr);

		if (!pTicker->isRegister())
			return;

		CCoreTickerNode* pCoreTickerNode = reinterpret_cast<CCoreTickerNode*>(pTicker->m_pCoreContext);
		pCoreTickerNode->Value.m_nState = eUnRegister;

		pCoreTickerNode->Value.m_pTicker = nullptr;
		pTicker->m_pCoreContext = nullptr;
		pCoreTickerNode->Value.addRef();

		std::unique_lock<base::spin_lock> lock(this->m_lockUnRegister);
		this->m_vecUnRegisterTicker.push_back(pCoreTickerNode);
	}

	void CTickerRunnable::insertTicker(CCoreTickerNode* pTickerNode)
	{
		DebugAst(pTickerNode->Value.m_nNextTime >= this->m_nLogicTime);

		int64_t nDeltaTime = pTickerNode->Value.m_nNextTime - this->m_nLogicTime;

		if (nDeltaTime < __TIME_NEAR_SIZE)
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
				if (nDeltaTime < nMask)
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
		std::vector<CCoreTickerNode*> vecSwapTicker;
		this->m_lockRegister.lock();
		std::swap(vecSwapTicker, this->m_vecRegisterTicker);
		this->m_lockRegister.unlock();
		size_t nSwapCount = vecSwapTicker.size();
		for (size_t i = 0; i < nSwapCount; ++i)
		{
			CCoreTickerNode* pCoreTickerNode = vecSwapTicker[i];
			if (pCoreTickerNode->Value.m_nNextTime < this->m_nLogicTime)
				pCoreTickerNode->Value.m_nNextTime = this->m_nLogicTime;
			this->insertTicker(pCoreTickerNode);
		}

		vecSwapTicker.clear();

		this->m_lockUnRegister.lock();
		std::swap(vecSwapTicker, this->m_vecUnRegisterTicker);
		this->m_lockUnRegister.unlock();
		nSwapCount = vecSwapTicker.size();
		for (size_t i = 0; i < nSwapCount; ++i)
		{
			CCoreTickerNode* pCoreTickerNode = vecSwapTicker[i];

			if (pCoreTickerNode->isLink())
			{
				pCoreTickerNode->remove();
				pCoreTickerNode->Value.release();
			}

			// 需要后释放，不然pCoreTickerNode可能已经释放了，pCoreTickerNode->isLink()也就未知了
			pCoreTickerNode->Value.release();
		}

		// 每一次更新都将刻度时间慢慢推进到与当前时间一样
		// 处理时间定时器
		for (; this->m_nLogicTime < nTime; ++this->m_nLogicTime)
		{
			// 先执行，如果此时正好有需要执行的定时器，也就能执行了，不然需要在cascadeTicker特殊处理以避免当前执行不到的bug
			this->cascadeTicker();

			uint32_t nPos = (uint32_t)(this->m_nLogicTime&__TIME_NEAR_MASK);
			auto& listTicker = this->m_listNearTicker[nPos];
			this->m_vecTempTickerNode.clear();

			while (!listTicker.empty())
			{
				CCoreTickerNode* pCoreTickerNode = listTicker.getHead();
				pCoreTickerNode->remove();

				this->onTicker(pCoreTickerNode);

				if (pCoreTickerNode->Value.m_nIntervalTime == 0)
					continue;

				pCoreTickerNode->Value.m_nNextTime += pCoreTickerNode->Value.m_nIntervalTime;
				this->m_vecTempTickerNode.push_back(pCoreTickerNode);
			}
			size_t nCount = this->m_vecTempTickerNode.size();
			for (size_t i = 0; i < nCount; ++i)
			{
				CCoreTickerNode* pCoreTickerNode = this->m_vecTempTickerNode[i];
				// 这段时间如果该定时器被反注册了，正常的反注册逻辑能够处理这种情况，这里不需要特殊处理，如果处理了会引起内存问题
// 				if (pCoreTickerNode->Value.m_nState == eUnRegister)
// 				{
// 					pCoreTickerNode->Value.release();
// 					continue;
// 				}

				this->insertTicker(pCoreTickerNode);
			}
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

					// 有可能正好一样，如果不加1就会执行不到了
// 					if (pTickerNode->Value.m_nNextTime == this->m_nLogicTime)
// 						pTickerNode->Value.m_nNextTime += 1;

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

				// 有可能正好一样，如果不加1就会执行不到了
// 				if (pTickerNode->Value.m_nNextTime == this->m_nLogicTime)
// 					pTickerNode->Value.m_nNextTime += 1;

				this->insertTicker(pTickerNode);
			}
		}
	}

	void CTickerRunnable::onTicker(CCoreTickerNode* pCoreTickerNode)
	{
		DebugAst(pCoreTickerNode != nullptr);

		if (pCoreTickerNode->Value.m_nIntervalTime != 0)
			pCoreTickerNode->Value.addRef();

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_TICKER;
		sMessagePacket.pData = pCoreTickerNode;
		sMessagePacket.nDataSize = 0;

		// 发送到消息队列，另外消息队列取出定时器对象的时候如果发现是一次性定时器，需要反注册
		pCoreTickerNode->Value.m_pMessageQueue->send(sMessagePacket);
	}

	CCoreTickerInfo::CCoreTickerInfo()
	{
		this->m_pTicker = nullptr;
		this->m_pMemory = nullptr;
		this->m_nNextTime = 0;
		this->m_nIntervalTime = 0;
		this->m_pMessageQueue = nullptr;
		this->m_nRef.store(0, std::memory_order_relaxed);
		this->m_nState = eRegister;
	}

	CCoreTickerInfo::~CCoreTickerInfo()
	{
		DebugAst(this->getRef() == 0);
	}

	void CCoreTickerInfo::addRef()
	{
		this->m_nRef.fetch_add(1, std::memory_order_relaxed);
	}

	void CCoreTickerInfo::release()
	{
		if ((this->m_nRef.fetch_sub(1, std::memory_order_acq_rel)) == 0)
		{
			CCoreTickerNode* pCoreTickerNode = reinterpret_cast<CCoreTickerNode*>(this->m_pMemory);
			SAFE_DELETE(pCoreTickerNode);
		}
	}

	int32_t CCoreTickerInfo::getRef() const
	{
		return this->m_nRef.load(std::memory_order_acquire);
	}
}