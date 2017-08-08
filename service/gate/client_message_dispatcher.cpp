#include "stdafx.h"
#include "client_message_dispatcher.h"
#include "gate_connection_from_client.h"
#include "gate_service.h"
#include "client_session.h"
#include "client_session_mgr.h"

#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/base_app.h"

using namespace core;

CClientMessageDispatcher::CClientMessageDispatcher(CGateService* pGateService)
	: m_pGateService(pGateService)
{

}

CClientMessageDispatcher::~CClientMessageDispatcher()
{
}

void CClientMessageDispatcher::registerMessageHandler(const std::string& szMessageName, const ClientCallback& callback)
{
	DebugAst(callback != nullptr);

	uint32_t nMessageID = base::hash(szMessageName.c_str());
	auto iter = this->m_mapMessageHandler.find(nMessageID);
	if (iter != this->m_mapMessageHandler.end())
	{
		PrintWarning("dup client message name message_name: %s", szMessageName.c_str());
		return;
	}

	SClientMessageHandler sClientMessageHandler;
	sClientMessageHandler.szMessageName = szMessageName;
	sClientMessageHandler.callback = callback;
	this->m_mapMessageHandler[nMessageID] = sClientMessageHandler;
}

void CClientMessageDispatcher::dispatch(CGateConnectionFromClient* pGateConnectionFromClient, const void* pData, uint16_t nSize)
{
	DebugAst(pData != nullptr);
	DebugAst(pGateConnectionFromClient != nullptr);

	const message_header* pHeader = reinterpret_cast<const message_header*>(pData);

	auto iter = this->m_mapMessageHandler.find(pHeader->nMessageID);
	if (iter == this->m_mapMessageHandler.end())
	{
		CClientSession* pClientSession = this->m_pGateService->getClientSessionMgr()->getSessionBySocketID(pGateConnectionFromClient->getID());
		if(pClientSession == nullptr || (pClientSession->getState()&eCSS_Normal) == 0)
		{
			pGateConnectionFromClient->shutdown(true, "invalid session");
			return;
		}

		this->forward(pClientSession, pHeader);
		return;
	}
	
	const char* pMessageData = reinterpret_cast<const char*>(pHeader + 1);
	const std::string& szMessageName = iter->second.szMessageName;

	google::protobuf::Message* pMessage = this->m_pGateService->getProtobufFactory()->unserialize_protobuf_message_from_buf(szMessageName, pMessageData, nSize - sizeof(message_header));
	if (nullptr == pMessage)
	{
		PrintWarning("unserialize_protobuf_message_from_buf error message_name: %s", szMessageName.c_str());
		return;
	}

	ClientCallback& callback = iter->second.callback;
	DebugAst(callback != nullptr);

	callback(pGateConnectionFromClient, pMessage);
}

void CClientMessageDispatcher::forward(CClientSession* pClientSession, const message_header* pHeader)
{
	DebugAst(pClientSession != nullptr && pHeader != nullptr);

	this->m_pGateService->getServiceInvoker()->gate_forward(pClientSession->getSessionID(), pClientSession->getSocketID(), pClientSession->getServiceID(), pClientSession->getPlayerID(), pHeader);
}