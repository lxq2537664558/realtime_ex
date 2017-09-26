#include "stdafx.h"
#include "ticker_runnable.h"
#include "core_common.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "core_app.h"
#include "message_command.h"
#include "net_runnable.h"
#include "logic_runnable.h"

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

		CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);

		int64_t nEndTime = base::time_util::getGmtTime();
		int64_t nDeltaTime = nEndTime - nCurTime;
		if (_CYCLE_TIME - nDeltaTime > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(_CYCLE_TIME - nDeltaTime));

		return true;
	}

	bool CTickerRunnable::registerTicker(uint8_t nType, uint32_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		DebugAstEx(pTicker != nullptr, false);
		DebugAstEx(!pTicker->isRegister(), false);
		DebugAstEx(pTicker->getCallback() != nullptr, false);

		DebugAstEx(nType == CTicker::eTT_Service || nType == CTicker::eTT_Net || nType == CTicker::eTT_Actor, false);

		CCoreTickerNode* pCoreTickerNode = new CCoreTickerNode();
		pCoreTickerNode->Value.m_pTicker = pTicker;
		pCoreTickerNode->Value.m_nNextTime = this->m_nLogicTime + nStartTime;
		pCoreTickerNode->Value.m_nIntervalTime = nIntervalTime;
		pCoreTickerNode->Value.m_pMemory = pCoreTickerNode;
		pCoreTickerNode->Value.m_nType = nType;
		pCoreTickerNode->Value.m_nState = eRegister;
		pCoreTickerNode->Value.m_nRef = 1;

		pTicker->m_nIntervalTime = nIntervalTime;
		pTicker->m_nContext = nContext;
		pTicker->m_pCoreContext = pCoreTickerNode;
		pTicker->m_nType = nType;
		pTicker->m_nServiceID = nFromServiceID;
		pTicker->m_nActorID = nFromActorID;

		std::unique_lock<base::spin_lock> lock(this->m_lock);
		this->m_vecSwapTicker.push_back(pCoreTickerNode);
		
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
	}

	void CTickerRunnable::insertTicker(CCoreTickerNode* pTickerNode)
	{
		DebugAst(pTickerNode->Value.m_nNextTime >= this->m_nLogicTime);

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
		std::vector<CCoreTickerNode*> vecSwapTicker;
		this->m_lock.lock();
		std::swap(vecSwapTicker, this->m_vecSwapTicker);
		this->m_lock.unlock();
		size_t nSwapCount = vecSwapTicker.size();
		for (size_t i = 0; i < nSwapCount; ++i)
		{
			CCoreTickerNode* pCoreTickerNode = vecSwapTicker[i];
			if (pCoreTickerNode->Value.m_nNextTime < this->m_nLogicTime)
				pCoreTickerNode->Value.m_nNextTime = this->m_nLogicTime;
			this->insertTicker(pCoreTickerNode);
		}
		
		// 每一次更新都将刻度时间慢慢推进到与当前时间一样
		// 处理时间定时器
		for (; this->m_nLogicTime < nTime; ++this->m_nLogicTime)
		{
			uint32_t nPos = (uint32_t)(this->m_nLogicTime&__TIME_NEAR_MASK);
			auto& listTicker = this->m_listNearTicker[nPos];
			this->m_vecTempTickerNode.clear();

			while (!listTicker.empty())
			{
				CCoreTickerNode* pCoreTickerNode = listTicker.getHead();
				pCoreTickerNode->remove();
				
				if (pCoreTickerNode->Value.m_nState != eRegister)
				{
					pCoreTickerNode->Value.release();
					continue;
				}
				
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
				if (pCoreTickerNode->Value.m_nState == eUnRegister)
				{
					pCoreTickerNode->Value.release();
					continue;
				}

				this->insertTicker(pCoreTickerNode);
			}

			this->cascadeTicker();
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
					if (pTickerNode->Value.m_nNextTime == this->m_nLogicTime)
						pTickerNode->Value.m_nNextTime += 1;

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
				if (pTickerNode->Value.m_nNextTime == this->m_nLogicTime)
					pTickerNode->Value.m_nNextTime += 1;

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
		if (pCoreTickerNode->Value.m_nType == CTicker::eTT_Service || pCoreTickerNode->Value.m_nType == CTicker::eTT_Actor)
			CCoreApp::Inst()->getLogicRunnable()->getMessageQueue()->send(sMessagePacket);
		else
			CCoreApp::Inst()->getNetRunnable()->getMessageQueue()->send(sMessagePacket);
	}

	CCoreTickerInfo::CCoreTickerInfo()
	{
		this->m_pTicker = nullptr;
		this->m_pMemory = nullptr;
		this->m_nNextTime = 0;
		this->m_nIntervalTime = 0;
		this->m_nType = CTicker::eTT_None;
		this->m_nRef = 0;
		this->m_nState = eRegister;
	}

	CCoreTickerInfo::~CCoreTickerInfo()
	{
		DebugAst(this->m_nRef == 0);
	}

	void CCoreTickerInfo::addRef()
	{
		++this->m_nRef;
	}

	void CCoreTickerInfo::release()
	{
		if ((--this->m_nRef) == 0)
		{
			CCoreTickerNode* pCoreTickerNode = reinterpret_cast<CCoreTickerNode*>(this->m_pMemory);
			SAFE_DELETE(pCoreTickerNode);
		}
	}

	int32_t CCoreTickerInfo::getRef() const
	{
		return this->m_nRef.load();
	}
}