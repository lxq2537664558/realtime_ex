#include "stdafx.h"
#include "message_dispatcher.h"
#include "core_common_define.h"
#include "message_command.h"
#include "coroutine.h"
#include "core_actor.h"
#include "core_app.h"
#include "core_service.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

#include <memory>

namespace core
{
	CMessageDispatcher::CMessageDispatcher(CCoreService* pCoreService)
		: m_pCoreService(pCoreService)
	{

	}

	CMessageDispatcher::~CMessageDispatcher()
	{

	}

	void CMessageDispatcher::dispatch(uint32_t nFromNodeID, uint8_t nMessageType, const void* pContext)
	{
		DebugAst(pContext != nullptr);

		if (nMessageType == eMT_REQUEST)
		{
			const SMCT_REQUEST* pRequestContext = reinterpret_cast<const SMCT_REQUEST*>(pContext);
			if (pRequestContext->nToActorID != 0)
			{
				google::protobuf::Message* pMessage = pRequestContext->pMessage;

				CCoreActor* pCoreActor = this->m_pCoreService->getActorScheduler()->getCoreActor(pRequestContext->nToActorID);
				if (nullptr == pCoreActor)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = pRequestContext->nFromActorID;
				sActorMessagePacket.nFromServiceID = pRequestContext->nFromServiceID;
				sActorMessagePacket.nSessionID = pRequestContext->nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.pMessage = pMessage;
				pCoreActor->getChannel()->send(sActorMessagePacket);

				this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
			}
			else
			{
				// 这里协程回调使用的pMessage的生命周期跟协程一致，采用值捕获lambda的方式来达到这一目的
				auto pMessage = std::shared_ptr<google::protobuf::Message>(pRequestContext->pMessage);

				auto& callback = this->m_pCoreService->getServiceMessageHandler(pMessage->GetTypeName());
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown request message service_id: {}, message_name: {}", this->m_pCoreService->getServiceID(), pMessage->GetTypeName());
					return;
				}

				SSessionInfo sSessionInfo;
				sSessionInfo.nFromServiceID = pRequestContext->nFromServiceID;
				sSessionInfo.nFromActorID = pRequestContext->nFromActorID;
				sSessionInfo.nSessionID = pRequestContext->nSessionID;

				uint64_t nCoroutineID = coroutine::create(0, [&callback, this, pMessage, sSessionInfo](uint64_t) { callback(this->m_pCoreService->getServiceBase(), sSessionInfo, pMessage.get()); });
				coroutine::resume(nCoroutineID, 0);
			}
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			const SMCT_RESPONSE* pResponseContext = reinterpret_cast<const SMCT_RESPONSE*>(pContext);
			if (pResponseContext->nToActorID != 0)
			{
				google::protobuf::Message* pMessage = pResponseContext->pMessage;

				CCoreActor* pCoreActor = this->m_pCoreService->getActorScheduler()->getCoreActor(pResponseContext->nToActorID);
				if (nullptr == pCoreActor)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				if (pResponseContext->nSessionID != pCoreActor->getSyncPendingResponseSessionID())
				{
					SActorMessagePacket sActorMessagePacket;
					sActorMessagePacket.nData = pResponseContext->nResult;
					sActorMessagePacket.nSessionID = pResponseContext->nSessionID;
					sActorMessagePacket.nFromServiceID = 0;
					sActorMessagePacket.nType = eMT_RESPONSE;
					sActorMessagePacket.pMessage = pMessage;
					pCoreActor->getChannel()->send(sActorMessagePacket);

					this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
				}
				else
				{
					pCoreActor->setSyncPendingResponseMessage(pResponseContext->nResult, pMessage);

					this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
				}
			}
			else
			{
				// 这里有暂存消息的需求，所以需要用shared_ptr
				auto pMessage = std::shared_ptr<google::protobuf::Message>(pResponseContext->pMessage);
				
				SPendingResponseInfo* pPendingResponseInfo = CCoreApp::Inst()->getLogicRunnable()->getTransporter()->getPendingResponseInfo(pResponseContext->nSessionID);
				if (nullptr == pPendingResponseInfo)
					return;

				defer([&]()
				{
					SAFE_DELETE(pPendingResponseInfo);
				});

				if (pPendingResponseInfo->nCoroutineID != 0)
				{
					// 这里不能直接传pMessage的地址
					SSyncCallResultInfo* pSyncCallResultInfo = new SSyncCallResultInfo();
					pSyncCallResultInfo->nResult = pResponseContext->nResult;
					pSyncCallResultInfo->pMessage = pMessage;
					coroutine::setLocalData(pPendingResponseInfo->nCoroutineID, "response", reinterpret_cast<uint64_t>(pSyncCallResultInfo));

					coroutine::resume(pPendingResponseInfo->nCoroutineID, 0);
				}
				else if (pPendingResponseInfo->callback != nullptr)
				{
					uint8_t nResult = pResponseContext->nResult;
					if (nResult == eRRT_OK)
					{
						uint64_t nCoroutineID = coroutine::create(0, [&pPendingResponseInfo, pMessage](uint64_t) { pPendingResponseInfo->callback(pMessage, eRRT_OK); });
						coroutine::resume(nCoroutineID, 0);
					}
					else
					{
						uint64_t nCoroutineID = coroutine::create(0, [&pPendingResponseInfo, nResult](uint64_t) { pPendingResponseInfo->callback(nullptr, nResult); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
				else
				{
					PrintWarning("invalid response session_id: {} service_id: {} message_name: {}", pPendingResponseInfo->nSessionID, this->m_pCoreService->getServiceID(), pPendingResponseInfo->szMessageName);
				}
			}
		}
		else if (nMessageType == eMT_TO_GATE)
		{
			auto& callback = this->m_pCoreService->getToGateMessageCallback();
			if (callback == nullptr)
				return;

			const SMCT_TO_GATE* pToGateContext = reinterpret_cast<const SMCT_TO_GATE*>(pContext);

			callback(pToGateContext->nSessionID, pToGateContext->pData, pToGateContext->nDataSize);
		}
		else if (nMessageType == eMT_TO_GATE_BROADCAST)
		{
			auto& callback = this->m_pCoreService->getToGateBroadcastMessageCallback();
			if (callback == nullptr)
				return;

			const SMCT_TO_GATE_BROADCAST* pToGateBroadcastContext = reinterpret_cast<const SMCT_TO_GATE_BROADCAST*>(pContext);
			const uint64_t* pSessionID = reinterpret_cast<const uint64_t*>(pToGateBroadcastContext->pData);
			const void* pData = pToGateBroadcastContext->pData + sizeof(uint64_t) * pToGateBroadcastContext->nSessionCount;

			callback(pSessionID, pToGateBroadcastContext->nSessionCount, pData, (uint16_t)(pToGateBroadcastContext->nDataSize - sizeof(uint64_t) * pToGateBroadcastContext->nSessionCount));
		}
		else if (nMessageType == eMT_GATE_FORWARD)
		{
			const SMCT_GATE_FORWARD* pGateForwardContext = reinterpret_cast<const SMCT_GATE_FORWARD*>(pContext);
			if (pGateForwardContext->nToActorID != 0)
			{
				google::protobuf::Message* pMessage = pGateForwardContext->pMessage;

				CCoreActor* pCoreActor = this->m_pCoreService->getActorScheduler()->getCoreActor(pGateForwardContext->nToActorID);
				if (nullptr == pCoreActor)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = 0;
				sActorMessagePacket.nFromServiceID = pGateForwardContext->nFromServiceID;
				sActorMessagePacket.nSessionID = pGateForwardContext->nSessionID;
				sActorMessagePacket.nType = eMT_GATE_FORWARD;
				sActorMessagePacket.pMessage = pMessage;
				pCoreActor->getChannel()->send(sActorMessagePacket);

				this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
			}
			else
			{
				auto pMessage = std::shared_ptr<google::protobuf::Message>(pGateForwardContext->pMessage);

				auto& callback = this->m_pCoreService->getServiceForwardHandler(pMessage->GetTypeName());
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown gate forward message service_id: {}, message_name: {}", this->m_pCoreService->getServiceID(), pMessage->GetTypeName());
					return;
				}

				SClientSessionInfo sClientSessionInfo;
				sClientSessionInfo.nSessionID = pGateForwardContext->nSessionID;
				sClientSessionInfo.nGateServiceID = pGateForwardContext->nFromServiceID;

				uint64_t nCoroutineID = coroutine::create(0, [&callback, this, pMessage, sClientSessionInfo](uint64_t) { callback(this->m_pCoreService->getServiceBase(), sClientSessionInfo, pMessage.get()); });
				coroutine::resume(nCoroutineID, 0);
			}
		}
	}
}