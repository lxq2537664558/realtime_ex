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
			// ����Ķ�ʱ��
			uint32_t nPos = (uint32_t)(pTickerNode->Value.m_nNextTime&__TIME_NEAR_MASK);
			this->m_listNearTicker[nPos].pushTail(pTickerNode);
		}
		else
		{
			// Զ�̵Ķ�ʱ�����ȼ����������һ������
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
				// ������������
				uint32_t nPos = (uint32_t)((pTickerNode->Value.m_nNextTime >> (__TIME_NEAR_BITS + nLevel * __TIME_CASCADE_BITS))&__TIME_CASCADE_MASK);
				DebugAst(nPos != 0);

				this->m_listCascadeTicker[nLevel][nPos].pushTail(pTickerNode);
			}
			else
			{
				// �޴�ĳ�ʱʱ�䣬ֱ�ӷŵ�Զ��������
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

			// ��Ҫ���ͷţ���ȻpCoreTickerNode�����Ѿ��ͷ��ˣ�pCoreTickerNode->isLink()Ҳ��δ֪��
			pCoreTickerNode->Value.release();
		}

		// ÿһ�θ��¶����̶�ʱ�������ƽ����뵱ǰʱ��һ��
		// ����ʱ�䶨ʱ��
		for (; this->m_nLogicTime < nTime; ++this->m_nLogicTime)
		{
			// ��ִ�У������ʱ��������Ҫִ�еĶ�ʱ����Ҳ����ִ���ˣ���Ȼ��Ҫ��cascadeTicker���⴦���Ա��⵱ǰִ�в�����bug
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
				// ���ʱ������ö�ʱ������ע���ˣ������ķ�ע���߼��ܹ�����������������ﲻ��Ҫ���⴦����������˻������ڴ�����
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
		// һ������һ�������������ң��ѷ��������Ķ�ʱ���������
		for (uint32_t nLevel = 0; nLevel < _countof(this->m_listCascadeTicker); ++nLevel)
		{
			// �ж��Ƿ��н�λ����
			if ((this->m_nLogicTime & (nMask - 1)) != 0)
				break;

			uint32_t nPos = nCascadeTime & __TIME_CASCADE_MASK;
			// �����0���϶���Ҫ�ټ�����һ��������
			if (nPos != 0)
			{
				auto& listTicker = this->m_listCascadeTicker[nLevel][nPos];
				while (!listTicker.empty())
				{
					CCoreTickerNode* pTickerNode = listTicker.getHead();
					pTickerNode->remove();

					// �п�������һ�����������1�ͻ�ִ�в�����
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

		// ������ǰ������������ֻ�ܱ������е�Far���������������ķǳ��󣬲����������ᷢ��
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

				// �п�������һ�����������1�ͻ�ִ�в�����
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

		// ���͵���Ϣ���У�������Ϣ����ȡ����ʱ�������ʱ�����������һ���Զ�ʱ������Ҫ��ע��
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