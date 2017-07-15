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
	CMessageDispatcher::CMessageDispatcher()
		: m_pServiceBaseImpl(nullptr)
	{

	}

	CMessageDispatcher::~CMessageDispatcher()
	{

	}

	bool CMessageDispatcher::init(CServiceBaseImpl* pServiceBaseImpl)
	{
		DebugAstEx(pServiceBaseImpl != nullptr, false);

		this->m_pServiceBaseImpl = pServiceBaseImpl;

		return true;
	}

	void CMessageDispatcher::dispatch(uint64_t nFromSocketID, uint16_t nFromNodeID, uint8_t nMessageType, const google::protobuf::Message* pMessage, const SMCT_RECV_SOCKET_DATA* pContext)
	{
		DebugAst(pMessage != nullptr && pContext != nullptr);

		if (nMessageType == eMT_REQUEST)
		{
			if (pContext->nTargetType == eMTT_Actor)
			{
				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(pContext->pData);

				CActorBaseImpl* pActorBase = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pContext->nToID);
				if (nullptr == pActorBase)
				{
					SAFE_DELETE(pMessage);
					return;
				}
				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = pContext->nFromID;
				sActorMessagePacket.nSessionID = pContext->nSessionID;
				sActorMessagePacket.nType = eMT_REQUEST;
				sActorMessagePacket.pMessage = pMessage;
				pActorBase->getChannel()->send(sActorMessagePacket);

				this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBase);
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
				sSessionInfo.nFromID = pContext->nFromID;
				sSessionInfo.nSessionID = pContext->nSessionID;
				callback(sSessionInfo, pMessage.get());
			}
		}
		else if (nMessageType == eMT_RESPONSE)
		{
			if (pContext->nTargetType == eMTT_Actor)
			{
				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(pContext->pData);

				CActorBaseImpl* pActorBase = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pContext->nToID);
				if (nullptr == pActorBase)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				if (pContext->nSessionID != pActorBase->getPendingResponseSessionID())
				{
					SActorMessagePacket sActorMessagePacket;
					sActorMessagePacket.nData = pContext->nFromID;
					sActorMessagePacket.nSessionID = pContext->nSessionID;
					sActorMessagePacket.nType = eMT_RESPONSE;
					sActorMessagePacket.pMessage = pMessage;
					pActorBase->getChannel()->send(sActorMessagePacket);

					this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBase);
				}
				else
				{
					pActorBase->setPendingResponseMessage((uint8_t)pContext->nFromID, pMessage);

					this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBase);
				}
			}
			else
			{
				auto pMessage = std::unique_ptr<google::protobuf::Message>(reinterpret_cast<google::protobuf::Message*>(pContext->pData));
				
				auto pPendingResponseInfo = std::unique_ptr<SPendingResponseInfo>(CCoreApp::Inst()->getTransporter()->getPendingResponseInfo(pContext->nSessionID, true));
				if (nullptr == pPendingResponseInfo)
					return;

				if (pContext->nFromID == eRRT_OK)
				{
					pPendingResponseInfo->callback(pMessage.get(), eRRT_OK);
				}
				else
				{
					pPendingResponseInfo->callback(nullptr, (EResponseResultType)pContext->nFromID);
				}
			}
		}
		else if (nMessageType == eMT_GATE_FORWARD)
		{
			if (pContext->nTargetType == eMTT_Actor)
			{
				google::protobuf::Message* pMessage = reinterpret_cast<google::protobuf::Message*>(pContext->pData);

				CActorBaseImpl* pActorBase = this->m_pServiceBaseImpl->getActorScheduler()->getActorBase(pContext->nToID);
				if (nullptr == pActorBase)
				{
					SAFE_DELETE(pMessage);
					return;
				}

				SActorMessagePacket sActorMessagePacket;
				sActorMessagePacket.nData = pContext->nFromID;
				sActorMessagePacket.nSessionID = pContext->nSessionID;
				sActorMessagePacket.nType = eMT_GATE_FORWARD;
				sActorMessagePacket.pMessage = pMessage;
				pActorBase->getChannel()->send(sActorMessagePacket);

				this->m_pServiceBaseImpl->getActorScheduler()->addWorkActorBase(pActorBase);
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
				sClientSessionInfo.nGateServiceID = (uint16_t)pContext->nFromID;
				sClientSessionInfo.nSessionID = pContext->nSessionID;

				callback(sClientSessionInfo, pMessage.get());
			}
		}
	}
}