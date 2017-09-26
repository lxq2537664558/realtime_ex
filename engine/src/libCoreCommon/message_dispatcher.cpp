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
			
			CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializerByType(pRequestContext->nMessageSerializerType);
			DebugAst(pMessageSerializer != nullptr);

			void* pMessage = pMessageSerializer->unserializeMessageFromBuf(pRequestContext->szMessageName, ((char*)pRequestContext) + sizeof(SMCT_REQUEST) + pRequestContext->nMessageNameLen, pRequestContext->nMessageDataLen);
			if (nullptr == pMessage)
			{
				PrintWarning("CMessageDispatcher::dispatch eMT_REQUEST error pMessage == nullptr service_id: {} message_name: {}", pRequestContext->nToServiceID, pRequestContext->szMessageName);
				return;
			}

			if (pRequestContext->nToActorID != 0)
			{
				CCoreActor* pCoreActor = this->m_pCoreService->getActorScheduler()->getCoreActor(pRequestContext->nToActorID);
				if (nullptr == pCoreActor)
				{
					pMessageSerializer->destroyMessage(pMessage);
					return;
				}

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = pRequestContext->nFromActorID;
				sActorMessagePacket.nFromServiceID = pRequestContext->nFromServiceID;
				sActorMessagePacket.nSessionID = pRequestContext->nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.nMessageSerializerType = pRequestContext->nMessageSerializerType;
				sActorMessagePacket.pMessage = pMessage;
				pCoreActor->getChannel()->send(sActorMessagePacket);

				this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
			}
			else
			{
				uint32_t nFromServiceID = pRequestContext->nFromServiceID;
				// 这里协程回调使用的pMessage的生命周期跟协程一致，采用值捕获lambda的方式来达到这一目的
				auto pMessagePtr = std::shared_ptr<void>(pMessage, [this, nFromServiceID](void* pRawMessage)
				{
					CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializer(nFromServiceID);
					DebugAst(pMessageSerializer != nullptr);

					pMessageSerializer->destroyMessage(pRawMessage);
				});

				auto& callback = this->m_pCoreService->getServiceMessageHandler(pRequestContext->szMessageName);
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown request message service_id: {}, message_name: {}", this->m_pCoreService->getServiceID(), pRequestContext->szMessageName);
					return;
				}

				SSessionInfo sSessionInfo;
				sSessionInfo.nFromServiceID = pRequestContext->nFromServiceID;
				sSessionInfo.nFromActorID = pRequestContext->nFromActorID;
				sSessionInfo.nSessionID = pRequestContext->nSessionID;

				uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&callback, this, pMessagePtr, sSessionInfo](uint64_t) { callback(this->m_pCoreService->getServiceBase(), sSessionInfo, pMessagePtr.get()); });
				coroutine::resume(nCoroutineID, 0);
			}
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			const SMCT_RESPONSE* pResponseContext = reinterpret_cast<const SMCT_RESPONSE*>(pContext);
			DebugAst(pResponseContext->nSessionID != 0);

			CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializerByType(pResponseContext->nMessageSerializerType);
			DebugAst(pMessageSerializer != nullptr);

			void* pMessage = nullptr;
			if (base::function_util::strnlen(pResponseContext->szMessageName, _TRUNCATE) != 0)
			{
				pMessage = pMessageSerializer->unserializeMessageFromBuf(pResponseContext->szMessageName, ((char*)pResponseContext) + sizeof(SMCT_RESPONSE) + pResponseContext->nMessageNameLen, pResponseContext->nMessageDataLen);
				if (nullptr == pMessage)
				{
					PrintWarning("CMessageDispatcher::dispatch eMT_RESPONSE error pMessage == nullptr service_id: {} message_name: {}", pResponseContext->nToServiceID, pResponseContext->szMessageName);
					return;
				}
			}

			if (pResponseContext->nToActorID != 0)
			{

				CCoreActor* pCoreActor = this->m_pCoreService->getActorScheduler()->getCoreActor(pResponseContext->nToActorID);
				if (nullptr == pCoreActor)
				{
					pMessageSerializer->destroyMessage(pMessage);
					return;
				}

				if (pResponseContext->nSessionID != pCoreActor->getSyncPendingResponseSessionID())
				{
					SActorMessagePacket sActorMessagePacket;
					sActorMessagePacket.nData = pResponseContext->nResult;
					sActorMessagePacket.nSessionID = pResponseContext->nSessionID;
					sActorMessagePacket.nFromServiceID = pResponseContext->nFromServiceID;
					sActorMessagePacket.nType = eMT_RESPONSE;
					sActorMessagePacket.nMessageSerializerType = pResponseContext->nMessageSerializerType;
					sActorMessagePacket.pMessage = pMessage;
					pCoreActor->getChannel()->send(sActorMessagePacket);

					this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
				}
				else
				{
					pCoreActor->setSyncPendingResponseMessage(pResponseContext->nResult, pMessage, pResponseContext->nMessageSerializerType);

					this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
				}
			}
			else
			{
				uint32_t nFromServiceID = pResponseContext->nFromServiceID;
				// 这里有暂存消息的需求，所以需要用shared_ptr
				auto pMessagePtr = std::shared_ptr<void>(pMessage, [this, nFromServiceID](void* pRawMessage)
				{
					CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializer(nFromServiceID);
					DebugAst(pMessageSerializer != nullptr);

					pMessageSerializer->destroyMessage(pRawMessage);
				});
				
				SPendingResponseInfo* pPendingResponseInfo = this->m_pCoreService->getPendingResponseInfo(pResponseContext->nSessionID);
				if (nullptr == pPendingResponseInfo)
					return;

				defer([&]()
				{
					SAFE_DELETE(pPendingResponseInfo);
				});

				this->m_pCoreService->updateServiceHealth(pPendingResponseInfo->nToServiceID, false);

				if (pPendingResponseInfo->nCoroutineID != 0)
				{
					// 这里不能直接传pMessage的地址
					SSyncCallResultInfo* pSyncCallResultInfo = new SSyncCallResultInfo();
					pSyncCallResultInfo->nResult = pResponseContext->nResult;
					pSyncCallResultInfo->pMessage = pMessagePtr;
					coroutine::setLocalData(pPendingResponseInfo->nCoroutineID, "response", reinterpret_cast<uint64_t>(pSyncCallResultInfo));

					coroutine::resume(pPendingResponseInfo->nCoroutineID, 0);
				}
				else if (pPendingResponseInfo->callback != nullptr)
				{
					uint8_t nResult = pResponseContext->nResult;
					if (nResult == eRRT_OK)
					{
						uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&pPendingResponseInfo, pMessagePtr](uint64_t) { pPendingResponseInfo->callback(pMessagePtr, eRRT_OK); });
						coroutine::resume(nCoroutineID, 0);
					}
					else
					{
						uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&pPendingResponseInfo, nResult](uint64_t) { pPendingResponseInfo->callback(nullptr, nResult); });
						coroutine::resume(nCoroutineID, 0);
					}
				}
				else
				{
					PrintWarning("invalid response session_id: {} service_id: {}", pPendingResponseInfo->nSessionID, this->m_pCoreService->getServiceID());
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
			
			CMessageSerializer* pMessageSerializer = this->m_pCoreService->getForwardMessageSerializer();
			DebugAst(pMessageSerializer != nullptr);

			const message_header* pHeader = reinterpret_cast<const message_header*>(pGateForwardContext + 1);
			const std::string& szMessageName = this->m_pCoreService->getForwardMessageName(pHeader->nMessageID);
			if (szMessageName.empty())
			{
				PrintWarning("CMessageDispatcher::dispatch eMT_GATE_FORWARD error szMessageName.empty() service_id: {} message_id: {}", this->m_pCoreService->getServiceID(), pHeader->nMessageID);
				return;
			}

			const char* pMessageData = reinterpret_cast<const char*>(pHeader + 1);

			void* pMessage = pMessageSerializer->unserializeMessageFromBuf(szMessageName, pMessageData, pGateForwardContext->nMessageDataLen);
			if (nullptr == pMessage)
			{
				PrintWarning("CMessageDispatcher::dispatch eMT_GATE_FORWARD error pMessage == nullptr service_id: {} message_name: {}", this->m_pCoreService->getServiceID(), szMessageName);
				return;
			}

			if (pGateForwardContext->nToActorID != 0)
			{
				CCoreActor* pCoreActor = this->m_pCoreService->getActorScheduler()->getCoreActor(pGateForwardContext->nToActorID);
				if (nullptr == pCoreActor)
				{
					pMessageSerializer->destroyMessage(pMessage);
					return;
				}

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = 0;
				sActorMessagePacket.nFromServiceID = pGateForwardContext->nFromServiceID;
				sActorMessagePacket.nSessionID = pGateForwardContext->nSessionID;
				sActorMessagePacket.nType = eMT_GATE_FORWARD;
				sActorMessagePacket.nMessageSerializerType = 0;
				sActorMessagePacket.pMessage = pMessage;
				pCoreActor->getChannel()->send(sActorMessagePacket);

				this->m_pCoreService->getActorScheduler()->addWorkCoreActor(pCoreActor);
			}
			else
			{
				auto pMessagePtr = std::shared_ptr<void>(pMessage, [this](void* pRawMessage)
				{
					CMessageSerializer* pMessageSerializer = this->m_pCoreService->getForwardMessageSerializer();
					DebugAst(pMessageSerializer != nullptr);

					pMessageSerializer->destroyMessage(pRawMessage);
				});

				auto& callback = this->m_pCoreService->getServiceForwardHandler(szMessageName);
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown gate forward message service_id: {}, message_name: {}", this->m_pCoreService->getServiceID(), szMessageName);
					return;
				}

				SClientSessionInfo sClientSessionInfo;
				sClientSessionInfo.nSessionID = pGateForwardContext->nSessionID;
				sClientSessionInfo.nGateServiceID = pGateForwardContext->nFromServiceID;

				uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&callback, this, pMessagePtr, sClientSessionInfo](uint64_t) { callback(this->m_pCoreService->getServiceBase(), sClientSessionInfo, pMessagePtr.get()); });
				coroutine::resume(nCoroutineID, 0);
			}
		}
	}
}