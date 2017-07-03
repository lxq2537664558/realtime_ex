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
			DebugAst(nSize > sizeof(request_cookice));

			const request_cookice* pCookice = reinterpret_cast<const request_cookice*>(pData);

			SServiceSessionInfo& sServiceSessionInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getServiceSessionInfo();
			sServiceSessionInfo.nServiceID = pCookice->nFromServiceID;
			sServiceSessionInfo.nSessionID = pCookice->nSessionID;

			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromNodeID);
				DebugAst(pSerializeAdapter != nullptr);
				
				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
				callback(nFromNodeID, pMessage);
			}
			sServiceSessionInfo.nServiceID = 0;
			sServiceSessionInfo.nSessionID = 0;
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_RESPONSE)
		{
			const response_cookice* pCookice = reinterpret_cast<const response_cookice*>(pData);
			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			
			SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->getResponseWaitInfo(pCookice->nSessionID, true);
			if (nullptr == pResponseWaitInfo)
				return;

			Defer(delete pResponseWaitInfo);

			if (pCookice->nResult == eRRT_OK)
			{
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromNodeID);
				DebugAst(pSerializeAdapter != nullptr);

				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
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
			// °þµôcookice
			const message_header* pHeader = reinterpret_cast<const message_header*>(pCookice + 1);
			
			SClientSessionInfo session(nFromNodeID, pCookice->nSessionID);

			auto& callback = CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->getGateForwardCallback(pHeader->nMessageID);
			if (callback != nullptr)
			{
				CSerializeAdapter* pSerializeAdapter = CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->getSerializeAdapter(nFromNodeID);
				DebugAst(pSerializeAdapter != nullptr);

				CMessagePtr<char> pMessage = pSerializeAdapter->deserialize(pHeader);
				callback(session, pMessage);
			}
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_REQUEST)
		{
			DebugAst(nSize > sizeof(core::actor_request_cookice));

			const core::actor_request_cookice* pCookice = reinterpret_cast<const core::actor_request_cookice*>(pData);

			// °þµôcookice
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);

			core::CActorBaseImpl* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(pCookice->nToActorID);
			if (NULL == pActorBase)
				return;

			char* pNewData = new char[nSize];
			memcpy(pNewData, pData, nSize);
			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = pCookice->nFromActorID;
			sMessagePacket.nType = eMT_ACTOR_REQUEST;
			sMessagePacket.nDataSize = nSize;
			sMessagePacket.pData = pNewData;
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_RESPONSE)
		{
			const core::actor_response_cookice* pCookice = reinterpret_cast<const core::actor_response_cookice*>(pData);
			// °þµôcookice
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);

			core::CActorBaseImpl* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(pCookice->nToActorID);
			if (NULL == pActorBase)
				return;

			char* pNewData = new char[nSize];
			memcpy(pNewData, pData, nSize);
			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = nFromNodeID;
			sMessagePacket.nType = eMT_RESPONSE;
			sMessagePacket.nDataSize = nSize;
			sMessagePacket.pData = pNewData;
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);
		}
		else if ((nMessageType&eMT_TYPE_MASK) == eMT_ACTOR_GATE_FORWARD)
		{
			const core::actor_gate_forward_cookice* pCookice = reinterpret_cast<const core::actor_gate_forward_cookice*>(pData);
			// °þµôcookice
			const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);

			core::CActorBaseImpl* pActorBase = core::CCoreServiceAppImpl::Inst()->getScheduler()->getBaseActor(pCookice->nToActorID);
			if (NULL == pActorBase)
				return;

			char* pNewData = new char[nSize];
			memcpy(pNewData, pData, nSize);
			core::SMessagePacket sMessagePacket;
			sMessagePacket.nID = nFromNodeID;
			sMessagePacket.nType = eMT_GATE_FORWARD;
			sMessagePacket.nDataSize = nSize;
			sMessagePacket.pData = pNewData;
			pActorBase->getChannel()->send(sMessagePacket);

			core::CCoreServiceAppImpl::Inst()->getScheduler()->addWorkActorBase(pActorBase);
		}
	}
}