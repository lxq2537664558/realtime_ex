#include "stdafx.h"
#include "gate_message_dispatcher.h"
#include "connection_from_client.h"
#include "gate_session.h"
#include "gate_app.h"

#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_kit.h"

CGateMessageDispatcher::CGateMessageDispatcher()
{

}

CGateMessageDispatcher::~CGateMessageDispatcher()
{

}

bool CGateMessageDispatcher::init()
{
	return true;
}

void CGateMessageDispatcher::registerCallback(const std::string& szMessageName, core::ClientCallback callback)
{
	DebugAst(callback != nullptr);

	uint32_t nMessageID = base::hash(szMessageName.c_str());
	auto iter = this->m_mapClientCallbackInfo.find(nMessageID);
	if (iter != this->m_mapClientCallbackInfo.end())
	{
		PrintWarning("dup client message name message_name: %s", szMessageName.c_str());
		return;
	}

	SClientCallbackInfo sClientCallbackInfo;
	sClientCallbackInfo.szMessageName = szMessageName;
	sClientCallbackInfo.callback = callback;
	this->m_mapClientCallbackInfo[nMessageID] = sClientCallbackInfo;
}

core::ClientCallback& CGateMessageDispatcher::getCallback(uint32_t nMessageID)
{
	auto iter = this->m_mapClientCallbackInfo.find(nMessageID);
	if (iter == this->m_mapClientCallbackInfo.end())
	{
		static core::ClientCallback s_callback;

		return s_callback;
	}

	return iter->second.callback;
}

core::ClientCallback& CGateMessageDispatcher::getCallback(const std::string& szMessageName)
{
	uint32_t nMessageID = base::hash(szMessageName.c_str());
	auto iter = this->m_mapClientCallbackInfo.find(nMessageID);
	if (iter == this->m_mapClientCallbackInfo.end())
	{
		static core::ClientCallback s_callback;

		return s_callback;
	}

	return iter->second.callback;
}

void CGateMessageDispatcher::dispatch(uint64_t nSocketID, uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(pData != nullptr);

	if ((nMessageType&eMT_TYPE_MASK) == eMT_CLIENT)
	{
		const core::client_message_header* pHeader = reinterpret_cast<const core::client_message_header*>(pData);

		auto iter = this->m_mapClientCallbackInfo.find(pHeader->nMessageID);
		if (iter == this->m_mapClientCallbackInfo.end())
		{
			CGateSession* pGateSession = CGateApp::Inst()->getGateSessionMgr()->getSessionBySocketID(nSocketID);
			if(pGateSession == nullptr)
			{
				core::CBaseConnection* pBaseConnection = CGateApp::Inst()->getBaseConnectionMgr()->getBaseConnection(nSocketID);
				DebugAst(pBaseConnection != nullptr);

				pBaseConnection->shutdown(true, "invalid session");
				return;
			}

			this->forward(pGateSession->getSessionID(), pHeader);
			return;
		}

		core::ClientCallback& callback = iter->second.callback;
		DebugAst(callback != nullptr);
		
		callback(nSocketID, pHeader);
	}
	else if ((nMessageType&eMT_TYPE_MASK) == eMT_TO_GATE)
	{
		DebugAst(nSize > sizeof(core::gate_cookice));

		const core::gate_cookice* pCookice = reinterpret_cast<const core::gate_cookice*>(pData);
		CGateSession* pGateSession = CGateApp::Inst()->getGateSessionMgr()->getSessionBySessionID(pCookice->nSessionID);
		if (nullptr == pGateSession)
		{
			PrintWarning("nullptr == pGateSession session_id: "UINT64FMT, pCookice->nSessionID);
			return;
		}
		
		core::CCoreServiceKit::Inst()->addTraceExtraInfo("trace_id: "UINT64FMT"send client", pCookice->nTraceID);

		core::CBaseConnection*  pBaseConnection = CGateApp::Inst()->getBaseConnectionMgr()->getBaseConnection(pGateSession->getSocketID());
		if (nullptr == pBaseConnection)
		{
			core::CCoreServiceKit::Inst()->addTraceExtraInfo("invalid connection client_id: "UINT64FMT" message_id: %d", pCookice->nSessionID, pCookice->nMessageID);
			return;
		}
		CConnectionFromClient* pConnectionFromClient = dynamic_cast<CConnectionFromClient*>(pBaseConnection);
		DebugAst(nullptr != pConnectionFromClient);

		int32_t nMessageSize = nSize - sizeof(core::gate_cookice);
		if (nMessageSize < 0)
		{
			core::CCoreServiceKit::Inst()->addTraceExtraInfo("invalid message size client_id: "UINT64FMT" message_id: %d message_size: %d", pCookice->nSessionID, pCookice->nMessageID, nMessageSize);
			return;
		}

		core::client_message_header header;
		header.nMessageID = pCookice->nMessageID;
		header.nMessageSize = (uint16_t)(nMessageSize + sizeof(core::client_message_header));
		pConnectionFromClient->send(eMT_CLIENT, &header, sizeof(core::client_message_header), pCookice + 1, (uint16_t)nMessageSize);
	}
}

void CGateMessageDispatcher::forward(uint64_t nSessionID, const core::client_message_header* pHeader)
{
	DebugAst(pHeader != nullptr);

	core::CClusterInvoker::Inst()->forward(pHeader->nMessageID, pHeader + 1, pHeader->nMessageSize - sizeof(core::client_message_header), nSessionID, "*");
}