#include "stdafx.h"
#include "gate_message_dispatcher.h"
#include "connection_from_client.h"
#include "gate_session.h"
#include "gate_app.h"

#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_app.h"

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
		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);

		auto iter = this->m_mapClientCallbackInfo.find(pHeader->nMessageID);
		if (iter == this->m_mapClientCallbackInfo.end())
		{
			CGateSession* pGateSession = CGateApp::Inst()->getGateSessionMgr()->getSessionBySocketID(nSocketID);
			if(pGateSession == nullptr)
			{
				core::CBaseConnection* pBaseConnection = CGateApp::Inst()->getBaseConnectionMgr()->getBaseConnectionByID(nSocketID);
				DebugAst(pBaseConnection != nullptr);

				pBaseConnection->shutdown(true, "invalid session");
				return;
			}

			this->forward(pGateSession->getSessionID(), pHeader);
			return;
		}

		core::ClientCallback& callback = iter->second.callback;
		DebugAst(callback != nullptr);
		
		core::CMessage pMessage(const_cast<core::message_header*>(pHeader));
		callback(nSocketID, pMessage);
	}
	else if ((nMessageType&eMT_TYPE_MASK) == eMT_TO_GATE)
	{
		DebugAst(nSize > sizeof(core::gate_send_cookice));

		const core::gate_send_cookice* pCookice = reinterpret_cast<const core::gate_send_cookice*>(pData);
		CGateSession* pGateSession = CGateApp::Inst()->getGateSessionMgr()->getSessionBySessionID(pCookice->nSessionID);
		if (nullptr == pGateSession)
		{
			PrintWarning("nullptr == pGateSession session_id: "UINT64FMT, pCookice->nSessionID);
			return;
		}

		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pCookice + 1);
		
		core::CCoreServiceApp::Inst()->addTraceExtraInfo("trace_id: "UINT64FMT"send client", pCookice->nTraceID);

		core::CBaseConnection*  pBaseConnection = CGateApp::Inst()->getBaseConnectionMgr()->getBaseConnectionByID(pGateSession->getSocketID());
		if (nullptr == pBaseConnection)
		{
			core::CCoreServiceApp::Inst()->addTraceExtraInfo("invalid connection client_id: "UINT64FMT" message_id: %d", pCookice->nSessionID, pHeader->nMessageID);
			return;
		}
		CConnectionFromClient* pConnectionFromClient = dynamic_cast<CConnectionFromClient*>(pBaseConnection);
		DebugAst(nullptr != pConnectionFromClient);

		pConnectionFromClient->send(eMT_CLIENT, pHeader, pHeader->nMessageSize);
	}
}

void CGateMessageDispatcher::forward(uint64_t nSessionID, const core::message_header* pHeader)
{
	DebugAst(pHeader != nullptr);

	core::cluster_invoker::forward("test1-1", nSessionID, pHeader);
}