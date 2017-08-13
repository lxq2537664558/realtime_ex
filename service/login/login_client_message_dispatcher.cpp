#include "stdafx.h"
#include "login_client_message_dispatcher.h"
#include "login_connection_from_client.h"
#include "login_service.h"

#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"

using namespace core;

CLoginClientMessageDispatcher::CLoginClientMessageDispatcher(CLoginService* pLoginService)
	: m_pLoginService(pLoginService)
{

}

CLoginClientMessageDispatcher::~CLoginClientMessageDispatcher()
{
}

void CLoginClientMessageDispatcher::registerMessageHandler(const std::string& szMessageName, const ClientCallback& callback)
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

void CLoginClientMessageDispatcher::dispatch(CLoginConnectionFromClient* pLoginConnectionFromClient, const void* pData, uint16_t nSize)
{
	DebugAst(pData != nullptr);
	DebugAst(pLoginConnectionFromClient != nullptr);

	const message_header* pHeader = reinterpret_cast<const message_header*>(pData);

	auto iter = this->m_mapMessageHandler.find(pHeader->nMessageID);
	if (iter == this->m_mapMessageHandler.end())
	{
		pLoginConnectionFromClient->shutdown(true, "unknown msg");
		return;
	}
	
	const char* pMessageData = reinterpret_cast<const char*>(pHeader + 1);
	const std::string& szMessageName = iter->second.szMessageName;

	google::protobuf::Message* pMessage = this->m_pLoginService->getForwardProtobufFactory()->unserialize_protobuf_message_from_buf(szMessageName, pMessageData, nSize - sizeof(message_header));
	if (nullptr == pMessage)
	{
		PrintWarning("unserialize_protobuf_message_from_buf error message_name: %s", szMessageName.c_str());
		return;
	}

	ClientCallback& callback = iter->second.callback;
	DebugAst(callback != nullptr);

	callback(pLoginConnectionFromClient, pMessage);
}