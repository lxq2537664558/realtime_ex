#include "stdafx.h"
#include "message_dispatcher.h"
#include "protobuf_helper.h"
#include "core_common_define.h"
#include "coroutine.h"
#include "actor_base_impl.h"
#include "core_app.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/defer.h"
#include <memory>

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
			
			const std::string szMessageName = pCookice->szMessageName;

			auto& callback = CCoreApp::Inst()->getCoreMessageRegistry()->getCallback(pCookice->nToServiceID, szMessageName);
			if (callback == nullptr)
			{
				PrintWarning("CMessageDispatcher::dispatch error unknown request message name %s", szMessageName.c_str());
				return;
			}

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			auto pMessage = std::unique_ptr<google::protobuf::Message>(create_protobuf_message(szMessageName));
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
				return;

			SServiceSessionInfo sServiceSessionInfo(pCookice->nFromServiceID, pCookice->nSessionID);

			callback(sServiceSessionInfo, pMessage.get());
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);

			DebugAst(nSize > sizeof(response_cookice));
			DebugAst(nSize > sizeof(response_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			auto pResponseWaitInfo = std::unique_ptr<SResponseWaitInfo>(CCoreApp::Inst()->getTransporter()->getResponseWaitInfo(pCookice->nSessionID, true));
			if (nullptr == pResponseWaitInfo)
				return;

			if (pCookice->nResult == eRRT_OK)
			{
				const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
				const std::string szMessageName = pCookice->szMessageName;
				auto pMessage = std::unique_ptr<google::protobuf::Message>(create_protobuf_message(szMessageName));
				if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
					return;

				pResponseWaitInfo->callback(pMessage.get(), eRRT_OK);
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

			const std::string szMessageName = pCookice->szMessageName;

			auto& callback = CCoreApp::Inst()->getCoreMessageRegistry()->getGateForwardCallback(pCookice->nToServiceID, szMessageName);
			if (callback == nullptr)
				return;

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			auto pMessage = std::unique_ptr<google::protobuf::Message>(create_protobuf_message(szMessageName));
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
				return;

			SClientSessionInfo session(nFromNodeID, pCookice->nSessionID);

			callback(session, pMessage.get());
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_REQUEST)
		{
			const actor_request_cookice* pCookice = reinterpret_cast<const actor_request_cookice*>(pData);

			DebugAst(nSize > sizeof(actor_request_cookice));
			DebugAst(nSize > sizeof(actor_request_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			CActorBaseImpl* pActorBase = CCoreApp::Inst()->getActorScheduler()->getActorBase(pCookice->nToActorID);
			if (nullptr == pActorBase)
				return;

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			SActorMessagePacket sActorMessagePacket;
			sActorMessagePacket.nData = pCookice->nFromActorID;
			sActorMessagePacket.nSessionID = pCookice->nSessionID;
			sActorMessagePacket.nType = eMT_ACTOR_REQUEST;
			sActorMessagePacket.pMessage = pMessage;
			pActorBase->getChannel()->send(sActorMessagePacket);

			CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(pActorBase);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_RESPONSE)
		{
			const actor_response_cookice* pCookice = reinterpret_cast<const actor_response_cookice*>(pData);

			DebugAst(nSize > sizeof(actor_response_cookice));
			DebugAst(nSize > sizeof(actor_response_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			CActorBaseImpl* pActorBase = CCoreApp::Inst()->getActorScheduler()->getActorBase(pCookice->nToActorID);
			if (nullptr == pActorBase)
				return;

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}
			
			SActorMessagePacket sActorMessagePacket;
			sActorMessagePacket.nData = pCookice->nResult;
			sActorMessagePacket.nSessionID = pCookice->nSessionID;
			sActorMessagePacket.nType = eMT_ACTOR_RESPONSE;
			sActorMessagePacket.pMessage = pMessage;
			pActorBase->getChannel()->send(sActorMessagePacket);

			CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(pActorBase);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_GATE_FORWARD)
		{
			const actor_gate_forward_cookice* pCookice = reinterpret_cast<const actor_gate_forward_cookice*>(pData);

			DebugAst(nSize > sizeof(actor_gate_forward_cookice));
			DebugAst(nSize > sizeof(actor_gate_forward_cookice) + pCookice->nMessageNameLen);
			DebugAst(pCookice->szMessageName[pCookice->nMessageNameLen] == 0);

			CActorBaseImpl* pActorBase = CCoreApp::Inst()->getActorScheduler()->getActorBase(pCookice->nToActorID);
			if (nullptr == pActorBase)
				return;

			const char* pMessageData = reinterpret_cast<const char*>(pCookice + 1) + pCookice->nMessageNameLen;
			const std::string szMessageName = pCookice->szMessageName;

			google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
			if (nullptr == pMessage || !pMessage->ParseFromArray(pMessageData, nSize - sizeof(request_cookice) - pCookice->nMessageNameLen))
			{
				SAFE_DELETE(pMessage);
				return;
			}

			SActorMessagePacket sActorMessagePacket;
			sActorMessagePacket.nData = pCookice->nFromServiceID;
			sActorMessagePacket.nSessionID = pCookice->nSessionID;
			sActorMessagePacket.nType = eMT_GATE_FORWARD;
			sActorMessagePacket.pMessage = pMessage;
			pActorBase->getChannel()->send(sActorMessagePacket);

			CCoreApp::Inst()->getActorScheduler()->addWorkActorBase(pActorBase);
		}
	}
}