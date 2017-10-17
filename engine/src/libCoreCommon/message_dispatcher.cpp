#include "stdafx.h"
#include "message_dispatcher.h"
#include "core_common_define.h"
#include "message_command.h"
#include "coroutine.h"
#include "core_app.h"
#include "core_service.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"
#include "libBaseCommon/profiling.h"

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

		CCoreApp::Inst()->incQPS();

		if (nMessageType == eMT_REQUEST)
		{
			PROFILING_GUARD(eMT_REQUEST)

			const SMCT_REQUEST* pRequestContext = reinterpret_cast<const SMCT_REQUEST*>(pContext);

			auto& callback = this->m_pCoreService->getServiceMessageHandler(pRequestContext->szMessageName);
			if (callback == nullptr)
			{
				PrintWarning("CMessageDispatcher::dispatch error unknown request message service_id: {}, message_name: {}", this->m_pCoreService->getServiceID(), pRequestContext->szMessageName);
				return;
			}

			CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializerByType(pRequestContext->nMessageSerializerType);
			DebugAst(pMessageSerializer != nullptr);

			const char* pMessageData = reinterpret_cast<const char*>(pRequestContext) + sizeof(SMCT_REQUEST) + pRequestContext->nMessageNameLen;
			void* pMessage = pMessageSerializer->unserializeMessageFromBuf(pRequestContext->szMessageName, pMessageData, pRequestContext->nMessageDataLen);
			if (nullptr == pMessage)
			{
				PrintWarning("CMessageDispatcher::dispatch eMT_REQUEST error pMessage == nullptr service_id: {} message_name: {}", this->m_pCoreService->getServiceID(), pRequestContext->szMessageName);
				return;
			}

			uint8_t nMessageSerializerType = pRequestContext->nMessageSerializerType;
			// 这里协程回调使用的pMessage的生命周期跟协程一致，采用值捕获lambda的方式来达到这一目的
			auto pMessagePtr = std::shared_ptr<void>(pMessage, [this, nMessageSerializerType](void* pRawMessage)
			{
				CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializerByType(nMessageSerializerType);
				DebugAst(pMessageSerializer != nullptr);

				pMessageSerializer->destroyMessage(pRawMessage);
			});

			SSessionInfo sSessionInfo;
			sSessionInfo.nFromServiceID = pRequestContext->nFromServiceID;
			sSessionInfo.nSessionID = pRequestContext->nSessionID;

			uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&callback, this, pMessagePtr, sSessionInfo](uint64_t) { callback(this->m_pCoreService->getServiceBase(), sSessionInfo, pMessagePtr.get()); });
			coroutine::resume(nCoroutineID, 0);
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			PROFILING_GUARD(eMT_RESPONSE)

			const SMCT_RESPONSE* pResponseContext = reinterpret_cast<const SMCT_RESPONSE*>(pContext);
			DebugAst(pResponseContext->nSessionID != 0);

			void* pMessage = nullptr;
			if (pResponseContext->nMessageNameLen != 0)
			{
				CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializerByType(pResponseContext->nMessageSerializerType);
				DebugAst(pMessageSerializer != nullptr);

				const char* pMessageData = reinterpret_cast<const char*>(pResponseContext) + sizeof(SMCT_RESPONSE) + pResponseContext->nMessageNameLen;
				pMessage = pMessageSerializer->unserializeMessageFromBuf(pResponseContext->szMessageName, pMessageData, pResponseContext->nMessageDataLen);
				if (nullptr == pMessage)
				{
					PrintWarning("CMessageDispatcher::dispatch eMT_RESPONSE error pMessage == nullptr service_id: {} message_name: {}", this->m_pCoreService->getServiceID(), pResponseContext->szMessageName);
					return;
				}
			}

			uint8_t nMessageSerializerType = pResponseContext->nMessageSerializerType;
			// 这里有暂存消息的需求，所以需要用shared_ptr
			auto pMessagePtr = std::shared_ptr<void>(pMessage, [this, nMessageSerializerType](void* pRawMessage)
			{
				if (pRawMessage == nullptr)
					return;

				CMessageSerializer* pMessageSerializer = this->m_pCoreService->getServiceMessageSerializerByType(nMessageSerializerType);
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

			DebugAst(pPendingResponseInfo->callback != nullptr);

			this->m_pCoreService->updateServiceHealth(pPendingResponseInfo->nToServiceID, pResponseContext->nResult == eRRT_TIME_OUT);

			if (pPendingResponseInfo->nCoroutineID == 0)
			{
				uint32_t nResult = pResponseContext->nResult;

				uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&pPendingResponseInfo, pMessagePtr, nResult](uint64_t) { pPendingResponseInfo->callback(pMessagePtr, nResult); });
				coroutine::resume(nCoroutineID, 0);
			}
			else
			{
				pPendingResponseInfo->callback(pMessagePtr, pResponseContext->nResult);
			}
		}
		else if (nMessageType == eMT_GATE_FORWARD)
		{
			PROFILING_GUARD(eMT_GATE_FORWARD)

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

			auto& callback = this->m_pCoreService->getServiceForwardHandler(szMessageName);
			if (callback == nullptr)
			{
				PrintWarning("CMessageDispatcher::dispatch error unknown gate forward message service_id: {}, message_name: {}", this->m_pCoreService->getServiceID(), szMessageName);
				return;
			}

			const char* pMessageData = reinterpret_cast<const char*>(pHeader + 1);

			void* pMessage = pMessageSerializer->unserializeMessageFromBuf(szMessageName, pMessageData, pHeader->nMessageSize - sizeof(message_header));
			if (nullptr == pMessage)
			{
				PrintWarning("CMessageDispatcher::dispatch eMT_GATE_FORWARD error pMessage == nullptr service_id: {} message_name: {}", this->m_pCoreService->getServiceID(), szMessageName);
				return;
			}

			auto pMessagePtr = std::shared_ptr<void>(pMessage, [this](void* pRawMessage)
			{
				CMessageSerializer* pMessageSerializer = this->m_pCoreService->getForwardMessageSerializer();
				DebugAst(pMessageSerializer != nullptr);

				pMessageSerializer->destroyMessage(pRawMessage);
			});

			SClientSessionInfo sClientSessionInfo;
			sClientSessionInfo.nSessionID = pGateForwardContext->nSessionID;
			sClientSessionInfo.nGateServiceID = pGateForwardContext->nFromServiceID;

			uint64_t nCoroutineID = coroutine::create(CCoreApp::Inst()->getCoroutineStackSize(), [&callback, this, pMessagePtr, sClientSessionInfo](uint64_t) { callback(this->m_pCoreService->getServiceBase(), sClientSessionInfo, pMessagePtr.get()); });
			coroutine::resume(nCoroutineID, 0);
		}
		else if (nMessageType == eMT_TO_GATE)
		{
			PROFILING_GUARD(eMT_TO_GATE)

			auto& callback = this->m_pCoreService->getToGateMessageCallback();
			if (callback == nullptr)
				return;

			const SMCT_TO_GATE* pToGateContext = reinterpret_cast<const SMCT_TO_GATE*>(pContext);

			callback(pToGateContext->nSessionID, pToGateContext->pData, pToGateContext->nDataSize);
		}
		else if (nMessageType == eMT_TO_GATE_BROADCAST)
		{
			PROFILING_GUARD(eMT_TO_GATE_BROADCAST)

			auto& callback = this->m_pCoreService->getToGateBroadcastMessageCallback();
			if (callback == nullptr)
				return;

			const SMCT_TO_GATE_BROADCAST* pToGateBroadcastContext = reinterpret_cast<const SMCT_TO_GATE_BROADCAST*>(pContext);
			const uint64_t* pSessionID = reinterpret_cast<const uint64_t*>(pToGateBroadcastContext->pData);
			const void* pData = pToGateBroadcastContext->pData + sizeof(uint64_t) * pToGateBroadcastContext->nSessionCount;

			callback(pSessionID, pToGateBroadcastContext->nSessionCount, pData, (uint16_t)(pToGateBroadcastContext->nDataSize - sizeof(uint64_t) * pToGateBroadcastContext->nSessionCount));
		}
	}
}