#include "stdafx.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_service_app_impl.h"
#include "core_service_kit_common.h"
#include "core_service_define.h"
#include "coroutine.h"
#include "actor_base_impl.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"

namespace core
{
	CMessageDispatcher::CMessageDispatcher()
	{

	}

	CMessageDispatcher::~CMessageDispatcher()
	{

	}

	bool CMessageDispatcher::init()
	{
		return true;
	}

	void CMessageDispatcher::dispatch(uint64_t nFromSocketID, uint16_t nFromNodeID, uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(pData != nullptr);

		if ((nMessageType&eMT_TYPE_MASK) == eMT_REQUEST)
		{
			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			DebugAst(nSize > sizeof(request_cookice));
			DebugAst(nSize > sizeof(request_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);
			
			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getCallback(pCookice->nToServiceID, szMessageName);
			if (callback == nullptr)
				return;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			SServiceSessionInfo sServiceSessionInfo(pCookice->nFromServiceID, pCookice->nSessionID);

			callback(sServiceSessionInfo, pMessage);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);

			DebugAst(nSize > sizeof(response_cookice));
			DebugAst(nSize > sizeof(response_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(pCookice->nSessionID, true);
			if (nullptr == pResponseWaitInfo)
				return;

			Defer(delete pResponseWaitInfo);

			if (pCookice->nResult == eRRT_OK)
			{
				const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
				const std::string szMessageName = pCookice->szMessageName;
				google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
				if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
				{
					SAFE_DELETE(pMessage);
					return;
				}

				pResponseWaitInfo->callback(pMessage, eRRT_OK);
			}
			else if (pCookice->nResult != eRRT_OK)
			{
				pResponseWaitInfo->callback(nullptr, (EResponseResultType)pCookice->nResult);
			}
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_GATE_FORWARD)
		{
			const gate_forward_cookice* pCookice = reinterpret_cast<const gate_forward_cookice*>(pData);

			DebugAst(nSize > sizeof(gate_forward_cookice));
			DebugAst(nSize > sizeof(gate_forward_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getGateForwardCallback(pCookice->nToServiceID, szMessageName);
			if (callback == nullptr)
				return;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			SClientSessionInfo session(nFromNodeID, pCookice->nSessionID);

			callback(session, pMessage);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_REQUEST)
		{
			const actor_request_cookice* pCookice = reinterpret_cast<const actor_request_cookice*>(pData);

			DebugAst(nSize > sizeof(actor_request_cookice));
			DebugAst(nSize > sizeof(actor_request_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			core::CActorBaseImpl* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(pCookice->nToActorID);
			if (NULL == pActorBase)
				return;

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = pCookice->nFromActorID;
			sMessagePacket.nType = eMT_ACTOR_REQUEST;
			sMessagePacket.pMessage = pMessage;
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_RESPONSE)
		{
			const actor_response_cookice* pCookice = reinterpret_cast<const actor_response_cookice*>(pData);

			DebugAst(nSize > sizeof(actor_response_cookice));
			DebugAst(nSize > sizeof(actor_response_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			core::CActorBaseImpl* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(pCookice->nToActorID);
			if (NULL == pActorBase)
				return;

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}
			
			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = 0;
			sMessagePacket.nType = eMT_ACTOR_RESPONSE;
			sMessagePacket.pMessage = pMessage;
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_GATE_FORWARD)
		{
			const actor_gate_forward_cookice* pCookice = reinterpret_cast<const actor_gate_forward_cookice*>(pData);

			DebugAst(nSize > sizeof(actor_gate_forward_cookice));
			DebugAst(nSize > sizeof(actor_gate_forward_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			core::CActorBaseImpl* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(pCookice->nToActorID);
			if (NULL == pActorBase)
				return;

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = nFromNodeID;
			sMessagePacket.nType = eMT_GATE_FORWARD;
			sMessagePacket.pMessage = pMessage;
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);
		}
	}
}