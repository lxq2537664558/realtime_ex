#include "gate_client_message_dispatcher.h"
#include "gate_connection_from_client.h"
#include "gate_service.h"
#include "gate_client_session.h"
#include "gate_client_session_mgr.h"

#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/coroutine.h"

using namespace core;

CGateClientMessageDispatcher::CGateClientMessageDispatcher(CGateService* pGateService)
	: m_pGateService(pGateService)
{
	this->m_pGateService->setToGateMessageCallback(std::bind(&CGateClientMessageDispatcher::onToGateMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	this->m_pGateService->setToGateBroadcastMessageCallback(std::bind(&CGateClientMessageDispatcher::onToGateBroadcastMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

CGateClientMessageDispatcher::~CGateClientMessageDispatcher()
{
}

void CGateClientMessageDispatcher::registerMessageHandler(const std::string& szMessageName, const ClientCallback& callback)
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

void CGateClientMessageDispatcher::dispatch(CGateConnectionFromClient* pGateConnectionFromClient, const void* pData, uint16_t nSize)
{
	DebugAst(pData != nullptr);
	DebugAst(pGateConnectionFromClient != nullptr);

	const message_header* pHeader = reinterpret_cast<const message_header*>(pData);

	auto iter = this->m_mapMessageHandler.find(pHeader->nMessageID);
	if (iter == this->m_mapMessageHandler.end())
	{
		CGateClientSession* pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->getSessionBySocketID(pGateConnectionFromClient->getID());
		if(pGateClientSession == nullptr || pGateClientSession->getState() != eCSS_Normal)
		{
			pGateConnectionFromClient->shutdown(true, "invalid session");
			return;
		}

		this->forward(pGateClientSession, pHeader);
		return;
	}
	
	const char* pMessageData = reinterpret_cast<const char*>(pHeader + 1);
	const std::string& szMessageName = iter->second.szMessageName;

	google::protobuf::Message* pMessage = static_cast<google::protobuf::Message*>(this->m_pGateService->getForwardMessageSerializer()->unserializeMessageFromBuf(szMessageName, pMessageData, nSize - sizeof(message_header)));
	if (nullptr == pMessage)
	{
		PrintWarning("unserializeMessageFromBuf error message_name: {}", szMessageName);
		return;
	}

	ClientCallback& callback = iter->second.callback;
	DebugAst(callback != nullptr);

	uint64_t nCoroutineID = core::coroutine::create(0, [&](uint64_t) { callback(pGateConnectionFromClient, pMessage); });
	core::coroutine::resume(nCoroutineID, 0);
}

void CGateClientMessageDispatcher::forward(CGateClientSession* pGateClientSession, const message_header* pHeader)
{
	DebugAst(pGateClientSession != nullptr && pHeader != nullptr);

	this->m_pGateService->getServiceInvoker()->gate_forward(pGateClientSession->getPlayerID(), pGateClientSession->getGasID(), pGateClientSession->getPlayerID(), pHeader);
}

void CGateClientMessageDispatcher::onToGateMessage(uint64_t nSessionID, const void* pData, uint16_t nDataSize)
{
	CGateClientSession* pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->getSessionByPlayerID(nSessionID);
	if (nullptr == pGateClientSession)
		return;

	if (pGateClientSession->getState() != eCSS_Normal)
		return;

	CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pGateClientSession->getSocketID());
	if (nullptr == pBaseConnection)
		return;

	pBaseConnection->send(eMT_CLIENT, pData, nDataSize);
}

void CGateClientMessageDispatcher::onToGateBroadcastMessage(const uint64_t* pSessionID, uint16_t nSessionCount, const void* pData, uint16_t nDataSize)
{
	for (uint16_t i = 0; i < nSessionCount; ++i)
	{
		CGateClientSession* pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->getSessionByPlayerID(pSessionID[i]);
		if (nullptr == pGateClientSession)
			return;

		if (pGateClientSession->getState() != eCSS_Normal)
			return;

		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pGateClientSession->getSocketID());
		if (nullptr == pBaseConnection)
			return;

		pBaseConnection->send(eMT_CLIENT, pData, nDataSize);
	}
}
