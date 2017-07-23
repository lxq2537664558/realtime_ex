#include "stdafx.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_common_define.h"
#include "coroutine.h"
#include "actor_base_impl.h"
#include "core_app.h"
#include "message_command.h"
#include "service_base_impl.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

#include <memory>

namespace core
{
	CMessageDispatcher::CMessageDispatcher(CServiceBaseImpl* pServiceBaseImpl)
		: m_pServiceBaseImpl(pServiceBaseImpl)
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

				CActorBaseImpl* pActorBaseImpl = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pRequestContext->nToActorID);
				if (nullptr == pActorBaseImpl)
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
				pActorBaseImpl->getChannel()->send(sActorMessagePacket);

				this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
			}
			else
			{
				// 这里没有暂存的需求，所以直接用unique_ptr
				auto pMessage = std::unique_ptr<google::protobuf::Message>(pRequestContext->pMessage);

				auto& callback = this->m_pServiceBaseImpl->getServiceMessageHandler(pMessage->GetTypeName());
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown request message service_id: %d, message_name: %s", this->m_pServiceBaseImpl->getServiceID(), pMessage->GetTypeName().c_str());
					return;
				}

				SSessionInfo sSessionInfo;
				sSessionInfo.eFromType = pRequestContext->nFromActorID != 0 ? eMTT_Actor : eMTT_Service;
				sSessionInfo.nFromServiceID = pRequestContext->nFromServiceID;
				sSessionInfo.nFromActorID = pRequestContext->nFromActorID;
				sSessionInfo.nSessionID = pRequestContext->nSessionID;
				callback(sSessionInfo, pMessage.get());
			}
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			const SMCT_RESPONSE* pResponseContext = reinterpret_cast<const SMCT_RESPONSE*>(pContext);
			if (pResponseContext->nToActorID != 0)
			{
				google::protobuf::Message* pMessage = pResponseContext->pMessage;

				CActorBaseImpl* pActorBaseImpl = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pResponseContext->nToActorID);
				if (nullptr == pActorBaseImpl)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				if (pResponseContext->nSessionID != pActorBaseImpl->getPendingResponseSessionID())
				{
					SActorMessagePacket sActorMessagePacket;
					sActorMessagePacket.nData = pResponseContext->nResult;
					sActorMessagePacket.nSessionID = pResponseContext->nSessionID;
					sActorMessagePacket.nFromServiceID = 0;
					sActorMessagePacket.nType = eMT_RESPONSE;
					sActorMessagePacket.pMessage = pMessage;
					pActorBaseImpl->getChannel()->send(sActorMessagePacket);

					this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
				}
				else
				{
					pActorBaseImpl->setPendingResponseMessage(pResponseContext->nResult, pMessage);

					this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
				}
			}
			else
			{
				// 这里有暂存消息的需求，所以需要用shared_ptr
				auto pMessage = std::shared_ptr<google::protobuf::Message>(pResponseContext->pMessage);
				
				auto pPendingResponseInfo = std::unique_ptr<SPendingResponseInfo>(CCoreApp::Inst()->getLogicRunnable()->getTransporter()->getPendingResponseInfo(pResponseContext->nSessionID, true));
				if (nullptr == pPendingResponseInfo)
					return;

				if (pResponseContext->nResult == eRRT_OK)
				{
					pPendingResponseInfo->callback(pMessage, eRRT_OK);
				}
				else
				{
					pPendingResponseInfo->callback(nullptr, (EResponseResultType)pResponseContext->nResult);
				}
			}
		}
		else if (nMessageType == eMT_GATE_FORWARD)
		{
			const SMCT_GATE_FORWARD* pGateForwardContext = reinterpret_cast<const SMCT_GATE_FORWARD*>(pContext);
			if (pGateForwardContext->nToActorID != 0)
			{
				google::protobuf::Message* pMessage = pGateForwardContext->pMessage;

				CActorBaseImpl* pActorBaseImpl = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pGateForwardContext->nToActorID);
				if (nullptr == pActorBaseImpl)
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
				pActorBaseImpl->getChannel()->send(sActorMessagePacket);

				this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
			}
			else
			{
				// 这里没有暂存的需求，所以直接用unique_ptr
				auto pMessage = std::unique_ptr<google::protobuf::Message>(pGateForwardContext->pMessage);

				auto& callback = this->m_pServiceBaseImpl->getServiceForwardHandler(pMessage->GetTypeName());
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown gate forward message service_id: %d, message_name: %s", this->m_pServiceBaseImpl->getServiceID(), pMessage->GetTypeName().c_str());
					return;
				}

				SClientSessionInfo sClientSessionInfo;
				sClientSessionInfo.nGateServiceID = pGateForwardContext->nFromServiceID;
				sClientSessionInfo.nSessionID = pGateForwardContext->nSessionID;

				callback(sClientSessionInfo, pMessage.get());
			}
		}
	}
}