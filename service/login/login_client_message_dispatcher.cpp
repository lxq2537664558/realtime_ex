#include "login_client_message_dispatcher.h"
#include "login_connection_from_client.h"
#include "login_service.h"

#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/coroutine.h"

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

	uint32_t nMessageID = _GET_MESSAGE_ID(szMessageName);
	auto iter = this->m_mapMessageHandler.find(nMessageID);
	if (iter != this->m_mapMessageHandler.end())
	{
		PrintWarning("dup client message name message_name: {}", szMessageName);
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

	google::protobuf::Message* pMessage = static_cast<google::protobuf::Message*>(this->m_pLoginService->getForwardMessageSerializer()->unserializeMessageFromBuf(szMessageName, pMessageData, nSize - sizeof(message_header)));
	if (nullptr == pMessage)
	{
		PrintWarning("unserializeMessageFromBuf error message_name: {}", szMessageName);
		return;
	}

	ClientCallback& callback = iter->second.callback;
	DebugAst(callback != nullptr);

	uint64_t nCoroutineID = core::coroutine::create(core::CBaseApp::Inst()->getCoroutineStackSize(), [&](uint64_t) { callback(pLoginConnectionFromClient, pMessage); });
	core::coroutine::resume(nCoroutineID, 0);
}