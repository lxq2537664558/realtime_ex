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

	void CMessageDispatcher::dispatch(uint64_t nFromSocketID, uint16_t nFromNodeID, const SMCT_RECV_SOCKET_DATA* pContext)
	{
		DebugAst(pContext != nullptr);
		uint8_t nMessageType = pContext->nMessageType;
		const google::protobuf::Message* pMessage = reinterpret_cast<const google::protobuf::Message*>(pContext->pData);
		DebugAst(pMessage);

		if (nMessageType == eMT_REQUEST)
		{
			if (pContext->nTargetType == eMTT_Actor)
			{
				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(pContext->pData);

				CActorBaseImpl* pActorBaseImpl = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pContext->nToActorID);
				if (nullptr == pActorBaseImpl)
				{
					SAFE_DELETE(pMessage);
					return;
				}
				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = pContext->nData;
				sActorMessagePacket.nSessionID = pContext->nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.pMessage = pMessage;
				pActorBaseImpl->getChannel()->send(sActorMessagePacket);

				this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
			}
			else
			{
				auto pMessage = std::unique_ptr<google::protobuf::Message>(reinterpret_cast<google::protobuf::Message*>(pContext->pData));

				auto& callback = this->m_pServiceBaseImpl->getServiceMessageHandler(pMessage->GetTypeName());
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown request message name %s", pMessage->GetTypeName().c_str());
					return;
				}

				SSessionInfo sSessionInfo;
				sSessionInfo.eTargetType = eMTT_Service;
				sSessionInfo.nFromID = pContext->nData;
				sSessionInfo.nSessionID = pContext->nSessionID;
				callback(sSessionInfo, pMessage.get());
			}
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			if (pContext->nTargetType == eMTT_Actor)
			{
				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(pContext->pData);

				CActorBaseImpl* pActorBaseImpl = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pContext->nToActorID);
				if (nullptr == pActorBaseImpl)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				if (pContext->nSessionID != pActorBaseImpl->getPendingResponseSessionID())
				{
					SActorMessagePacket sActorMessagePacket;
					sActorMessagePacket.nData = pContext->nData;
					sActorMessagePacket.nSessionID = pContext->nSessionID;
					sActorMessagePacket.nType = eMT_RESPONSE;
					sActorMessagePacket.pMessage = pMessage;
					pActorBaseImpl->getChannel()->send(sActorMessagePacket);

					this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
				}
				else
				{
					pActorBaseImpl->setPendingResponseMessage((uint8_t)pContext->nData, pMessage);

					this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
				}
			}
			else
			{
				auto pMessage = std::unique_ptr<google::protobuf::Message>(reinterpret_cast<google::protobuf::Message*>(pContext->pData));
				
				auto pPendingResponseInfo = std::unique_ptr<SPendingResponseInfo>(CCoreApp::Inst()->getTransporter()->getPendingResponseInfo(pContext->nSessionID, true));
				if (nullptr == pPendingResponseInfo)
					return;

				if (pContext->nData == eRRT_OK)
				{
					pPendingResponseInfo->callback(pMessage.get(), eRRT_OK);
				}
				else
				{
					pPendingResponseInfo->callback(nullptr, (EResponseResultType)pContext->nData);
				}
			}
		}
		else if (nMessageType == eMT_GATE_FORWARD)
		{
			if (pContext->nTargetType == eMTT_Actor)
			{
				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(pContext->pData);

				CActorBaseImpl* pActorBaseImpl = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pContext->nToActorID);
				if (nullptr == pActorBaseImpl)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = pContext->nData;
				sActorMessagePacket.nSessionID = pContext->nSessionID;
				sActorMessagePacket.nType = eMT_GATE_FORWARD;
				sActorMessagePacket.pMessage = pMessage;
				pActorBaseImpl->getChannel()->send(sActorMessagePacket);

				this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBaseImpl);
			}
			else
			{
				auto pMessage = std::unique_ptr<google::protobuf::Message>(reinterpret_cast<google::protobuf::Message*>(pContext->pData));

				auto& callback = this->m_pServiceBaseImpl->getServiceForwardHandler(pMessage->GetTypeName());
				if (callback == nullptr)
				{
					PrintWarning("CMessageDispatcher::dispatch error unknown gate forward message name %s", pMessage->GetTypeName().c_str());
					return;
				}

				SClientSessionInfo sClientSessionInfo;
				sClientSessionInfo.nGateServiceID = (uint16_t)pContext->nData;
				sClientSessionInfo.nSessionID = pContext->nSessionID;

				callback(sClientSessionInfo, pMessage.get());
			}
		}
	}
}