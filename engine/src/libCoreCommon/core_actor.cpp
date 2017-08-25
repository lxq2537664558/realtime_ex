#include "stdafx.h"
#include "core_actor.h"
#include "actor_base.h"
#include "service_base.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_service.h"
#include "ticker_runnable.h"

#include "libCoreCommon/base_app.h"
#include "libBaseCommon/defer.h"
#include "libBaseCommon/time_util.h"

#define _DEFAULT_CHANNEL_CAP 256

namespace core
{
	CCoreActor::CCoreActor(uint64_t nID, CActorBase* pActorBase, CCoreService* pCoreService)
		: m_channel(_DEFAULT_CHANNEL_CAP)
		, m_nID(nID)
		, m_pActorBase(pActorBase)
		, m_pCoreService(pCoreService)
		, m_pSyncPendingResponseMessage(nullptr)
		, m_pSyncPendingResponseInfo(nullptr)
		, m_nSyncPendingResponseHolderID(0)
		, m_nSyncPendingResponseResult(0)
		, m_eState(eABS_Normal)
	{

	}

	CCoreActor::~CCoreActor()
	{
		SAFE_DELETE(this->m_pSyncPendingResponseMessage);

		while (true)
		{
			SActorMessagePacket sActorMessagePacket;
			if (!this->m_channel.recv(sActorMessagePacket))
				break;

			SAFE_DELETE(sActorMessagePacket.pMessage);
		}
	}

	uint64_t CCoreActor::getID() const
	{
		return this->m_nID;
	}

	CCoreActor::EActorBaseState CCoreActor::getState() const
	{
		return this->m_eState;
	}

	void CCoreActor::setState(EActorBaseState eState)
	{
		this->m_eState = eState;
	}

	CCoreService* CCoreActor::getCoreService() const
	{
		return this->m_pCoreService;
	}

	void CCoreActor::process()
	{
		if (this->m_eState == eABS_RecvPending)
		{
			this->m_eState = eABS_Normal;

			SActorMessagePacket sActorMessagePacket;
			sActorMessagePacket.nData = this->m_nSyncPendingResponseResult;
			sActorMessagePacket.nSessionID = this->getSyncPendingResponseSessionID();
			sActorMessagePacket.nType = eMT_RESPONSE;
			sActorMessagePacket.pMessage = this->m_pSyncPendingResponseMessage;

			this->dispatch(sActorMessagePacket);

			this->m_pSyncPendingResponseMessage = nullptr;
			this->m_nSyncPendingResponseResult = 0;
		}

		while (this->m_eState == eABS_Normal)
		{
			SActorMessagePacket sActorMessagePacket;
			if (!this->m_channel.recv(sActorMessagePacket))
				break;

			this->dispatch(sActorMessagePacket);
		}
	}

	void CCoreActor::dispatch(const SActorMessagePacket& sActorMessagePacket)
	{
		// 这里协程回调使用的pMessage的生命周期跟协程一致，采用值捕获lambda的方式来达到这一目的
		if (sActorMessagePacket.nType == eMT_REQUEST)
		{
			auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);
			auto callback = this->m_pCoreService->getActorMessageHandler(pMessage->GetTypeName());
			if (callback == nullptr)
			{
				PrintWarning("CCoreActor::dispatch error unknown request message actor_id: {}, message_name: {}", this->getID(), pMessage->GetTypeName());
				return;
			}

			SSessionInfo sSessionInfo;
			sSessionInfo.eFromType = sActorMessagePacket.nData != 0 ? eMTT_Actor : eMTT_Service;
			sSessionInfo.nFromServiceID = sActorMessagePacket.nFromServiceID;
			sSessionInfo.nFromActorID = sActorMessagePacket.nData;
			sSessionInfo.nSessionID = sActorMessagePacket.nSessionID;

			uint64_t nCoroutineID = coroutine::create(0, [&callback, this, pMessage, sSessionInfo](uint64_t){ callback(this->m_pActorBase, sSessionInfo, pMessage.get()); });
			coroutine::resume(nCoroutineID, 0);
		}
		else if (sActorMessagePacket.nType == eMT_RESPONSE)
		{
			auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);
			SPendingResponseInfo* pPendingResponseInfo = this->getPendingResponseInfo(sActorMessagePacket.nSessionID);
			defer([&]() 
			{
				SAFE_DELETE(pPendingResponseInfo);
			});

			if (nullptr != pPendingResponseInfo)
			{
				if (pPendingResponseInfo->nCoroutineID != 0)
				{
					// 这里不能直接传pMessage的地址
					SSyncCallResultInfo* pSyncCallResultInfo = new SSyncCallResultInfo();
					pSyncCallResultInfo->nResult = (uint8_t)sActorMessagePacket.nData;
					pSyncCallResultInfo->pMessage = pMessage;
					coroutine::setLocalData(pPendingResponseInfo->nCoroutineID, "response", reinterpret_cast<uint64_t>(pSyncCallResultInfo));
					
					coroutine::resume(pPendingResponseInfo->nCoroutineID, 0);
				}
				else if (pPendingResponseInfo->callback != nullptr)
				{
					uint8_t nResult = (uint8_t)sActorMessagePacket.nData;
					if (nResult == eRRT_OK)
					{
						uint64_t nCoroutineID = coroutine::create(0, [&pPendingResponseInfo, pMessage, nResult](uint64_t){ pPendingResponseInfo->callback(pMessage, nResult); });
						coroutine::resume(nCoroutineID, 0);
					}
					else
					{
						uint64_t nCoroutineID = coroutine::create(0, [&pPendingResponseInfo, nResult](uint64_t){ pPendingResponseInfo->callback(nullptr, nResult); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
				else
				{
					PrintWarning("invalid response session_id: {} actor_id: {} message_name: {}", pPendingResponseInfo->nSessionID, this->getID(), pPendingResponseInfo->szMessageName);
				}
			}
		}
		else if (sActorMessagePacket.nType == eMT_GATE_FORWARD)
		{
			auto pMessage = std::shared_ptr<google::protobuf::Message>(sActorMessagePacket.pMessage);

			auto callback = this->m_pCoreService->getActorForwardHandler(pMessage->GetTypeName());
			if (callback == nullptr)
			{
				PrintWarning("CCoreActor::dispatch error unknown gate forawrd message actor_id: {}, message_name: {}", this->getID(), pMessage->GetTypeName());
				return;
			}
			
			SClientSessionInfo sClientSessionInfo;
			sClientSessionInfo.nSessionID = sActorMessagePacket.nSessionID;
			sClientSessionInfo.nGateServiceID = sActorMessagePacket.nFromServiceID;
			
			uint64_t nCoroutineID = coroutine::create(0, [&callback, this, pMessage, sClientSessionInfo](uint64_t){ callback(this->m_pActorBase, sClientSessionInfo, pMessage.get()); });
			coroutine::resume(nCoroutineID, 0);
		}
		else if (sActorMessagePacket.nType == eMT_TICKER)
		{
			CCoreTickerNode* pCoreTickerNode = reinterpret_cast<CCoreTickerNode*>(sActorMessagePacket.nSessionID);
			if (pCoreTickerNode == nullptr)
			{
				PrintWarning("CCoreActor::dispatch error pCoreTickerNode == nullptr actor_id: {}", this->getID());
				return;
			}

			if (pCoreTickerNode->Value.m_pTicker == nullptr)
			{
				pCoreTickerNode->Value.release();
				return;
			}

			uint64_t nCoroutineID = coroutine::create(0, [pCoreTickerNode](uint64_t)
			{
				CTicker* pTicker = pCoreTickerNode->Value.m_pTicker;
				pTicker->getCallback()(pTicker->getContext());
				pCoreTickerNode->Value.release();
			});
			coroutine::resume(nCoroutineID, 0);
		}
	}

	CChannel* CCoreActor::getChannel()
	{
		return &this->m_channel;
	}

	uint64_t CCoreActor::getSyncPendingResponseSessionID() const
	{
		if (this->m_pSyncPendingResponseInfo == nullptr)
			return 0;

		return this->m_pSyncPendingResponseInfo->nSessionID;
	}

	void CCoreActor::setSyncPendingResponseMessage(uint8_t nResult, google::protobuf::Message* pMessage)
	{
		DebugAst(this->m_eState == eABS_Pending);

		this->m_nSyncPendingResponseResult = nResult;
		this->m_pSyncPendingResponseMessage = pMessage;

		this->m_eState = eABS_RecvPending;
	}

	void CCoreActor::onRequestMessageTimeout(uint64_t nContext)
	{
		auto iter = this->m_mapPendingResponseInfo.find(nContext);
		if (iter == this->m_mapPendingResponseInfo.end())
		{
			PrintWarning("iter == this->m_mapResponseWaitInfo.end() session_id: {}", nContext);
			return;
		}

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		if (nullptr == pPendingResponseInfo)
		{
			PrintWarning("nullptr == pPendingResponseInfo session_id: {}", nContext);
			return;
		}

		SActorMessagePacket sActorMessagePacket;
		sActorMessagePacket.nData = eRRT_TIME_OUT;
		sActorMessagePacket.nSessionID = pPendingResponseInfo->nSessionID;
		sActorMessagePacket.nType = eMT_RESPONSE;
		sActorMessagePacket.pMessage = nullptr;

		this->m_channel.send(sActorMessagePacket);
	}

	SPendingResponseInfo* CCoreActor::addPendingResponseInfo(uint64_t nSessionID, uint64_t nCoroutineID, uint64_t nToID, const std::string& szMessageName, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, uint64_t nHolderID)
	{
		if (nCoroutineID == 0)
		{
			auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
			DebugAstEx(iter == this->m_mapPendingResponseInfo.end(), nullptr);

			SPendingResponseInfo* pPendingResponseInfo = new SPendingResponseInfo();
			pPendingResponseInfo->callback = callback;
			pPendingResponseInfo->nSessionID = nSessionID;
			pPendingResponseInfo->nCoroutineID = nCoroutineID;
			pPendingResponseInfo->nToID = nToID;
			pPendingResponseInfo->szMessageName = szMessageName;
			pPendingResponseInfo->nBeginTime = base::time_util::getGmtTime();
			pPendingResponseInfo->tickTimeout.setCallback(std::bind(&CCoreActor::onRequestMessageTimeout, this, std::placeholders::_1));
			this->getCoreService()->registerTicker(&pPendingResponseInfo->tickTimeout, CCoreApp::Inst()->getInvokeTimeout(), 0, nSessionID);

			this->m_mapPendingResponseInfo[pPendingResponseInfo->nSessionID] = pPendingResponseInfo;

			if (nHolderID != 0)
			{
				std::list<uint64_t>& listSessionID = this->m_mapHolderSessionIDList[nHolderID];
				listSessionID.push_back(nSessionID);
				pPendingResponseInfo->iterHolder = (--listSessionID.end());
				pPendingResponseInfo->nHolderID = nHolderID;
			}

			return pPendingResponseInfo;
		}
		else
		{
			if (nCoroutineID != 0 && this->m_pSyncPendingResponseInfo != nullptr)
			{
				DebugAstEx(!"nCoroutineID != 0 && this->m_pSyncPendingResponseInfo != nullptr", false);
			}

			this->m_pSyncPendingResponseInfo = new SPendingResponseInfo();
			this->m_pSyncPendingResponseInfo->callback = nullptr;
			this->m_pSyncPendingResponseInfo->nSessionID = nSessionID;
			this->m_pSyncPendingResponseInfo->nCoroutineID = nCoroutineID;
			this->m_pSyncPendingResponseInfo->nToID = nToID;
			this->m_pSyncPendingResponseInfo->szMessageName = szMessageName;
			this->m_pSyncPendingResponseInfo->nBeginTime = base::time_util::getGmtTime();

			this->m_nSyncPendingResponseHolderID = nHolderID;

			this->m_pCoreService->getActorScheduler()->addPendingCoreActor(this);

			return this->m_pSyncPendingResponseInfo;
		}
	}

	SPendingResponseInfo* CCoreActor::getPendingResponseInfo(uint64_t nSessionID)
	{
		if (this->m_pSyncPendingResponseInfo != nullptr && this->m_pSyncPendingResponseInfo->nSessionID == nSessionID)
		{
			SPendingResponseInfo* pPendingResponseInfo = this->m_pSyncPendingResponseInfo;
			this->m_pSyncPendingResponseInfo = nullptr;
			this->m_nSyncPendingResponseHolderID = 0;

			return pPendingResponseInfo;
		}

		auto iter = this->m_mapPendingResponseInfo.find(nSessionID);
		if (iter == this->m_mapPendingResponseInfo.end())
			return nullptr;

		SPendingResponseInfo* pPendingResponseInfo = iter->second;
		this->m_mapPendingResponseInfo.erase(iter);
		if (pPendingResponseInfo->nHolderID != 0)
		{
			auto iter = this->m_mapHolderSessionIDList.find(pPendingResponseInfo->nHolderID);
			if (iter != this->m_mapHolderSessionIDList.end())
			{
				std::list<uint64_t>& listSessionID = iter->second;
				listSessionID.erase(pPendingResponseInfo->iterHolder);
			}
		}

		return pPendingResponseInfo;
	}

	void CCoreActor::delPendingResponseInfo(uint64_t nHolderID)
	{
		if (this->m_nSyncPendingResponseHolderID == nHolderID)
		{
			this->m_nSyncPendingResponseHolderID = 0;
			SAFE_DELETE(this->m_pSyncPendingResponseInfo);
			this->m_eState = eABS_Normal;
		}

		auto iter = this->m_mapHolderSessionIDList.find(nHolderID);
		if (iter == this->m_mapHolderSessionIDList.end())
			return;

		std::list<uint64_t>& listSessionID = iter->second;
		for (auto iter = listSessionID.begin(); iter != listSessionID.end(); ++iter)
		{
			auto iterPendingResponseInfo = this->m_mapPendingResponseInfo.find(*iter);
			if (iterPendingResponseInfo == this->m_mapPendingResponseInfo.end())
				continue;

			SPendingResponseInfo* pPendingResponseInfo = iterPendingResponseInfo->second;
			this->m_mapPendingResponseInfo.erase(iterPendingResponseInfo);

			SAFE_DELETE(pPendingResponseInfo);
		}

		this->m_mapHolderSessionIDList.erase(iter);
	}

	bool CCoreActor::onPendingTimer(int64_t nCurTime)
	{
		if (this->m_pSyncPendingResponseInfo == nullptr)
			return false;

		if (nCurTime - this->m_pSyncPendingResponseInfo->nBeginTime < CCoreApp::Inst()->getInvokeTimeout())
			return false;
		
		this->setSyncPendingResponseMessage(eRRT_TIME_OUT, nullptr);
		
		this->m_pCoreService->getActorScheduler()->addWorkCoreActor(this);

		return true;
	}

	CActorBase* CCoreActor::getActorBase() const
	{
		return this->m_pActorBase;
	}
}