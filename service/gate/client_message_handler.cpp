#include "stdafx.h"
#include "client_message_handler.h"
#include "client_message_dispatcher.h"
#include "gate_service.h"

#include "libBaseCommon\token_parser.h"
#include "libBaseCommon\rand_gen.h"
#include "libCoreCommon\base_connection.h"
#include "libCoreCommon\base_connection_mgr.h"
#include "libCoreCommon\base_app.h"
#include "libCoreCommon\service_base.h"

#include "proto_src\gate_handshake_request.pb.h"
#include "proto_src\player_enter_request.pb.h"
#include "proto_src\player_enter_response.pb.h"
#include "proto_src\gate_handshake_response.pb.h"

using namespace core;

CClientMessageHandler::CClientMessageHandler(CGateService*	pGateService)
	: m_pGateService(pGateService)
{
	this->m_szBuf.resize(UINT16_MAX);

	this->m_pGateService->getClientMessageDispatcher()->registerMessageHandler("gate_handshake_request", std::bind(&CClientMessageHandler::handshake, this, std::placeholders::_1, std::placeholders::_2));
}

CClientMessageHandler::~CClientMessageHandler()
{

}

void CClientMessageHandler::sendClientMessage(CBaseConnection* pBaseConnection, const google::protobuf::Message* pMessage)
{
	DebugAst(pMessage != nullptr && pBaseConnection != nullptr);

	message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

	int32_t nDataSize = this->m_pGateService->getProtobufFactory()->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + sizeof(message_header), (uint32_t)(this->m_szBuf.size() - sizeof(message_header)));
	if (nDataSize < 0)
		return;

	pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
	pHeader->nMessageID = base::hash(pMessage->GetTypeName().c_str());

	pBaseConnection->send(eMT_CLIENT, &this->m_szBuf[0], pHeader->nMessageSize);
}

void CClientMessageHandler::handshake(CConnectionFromClient* pConnectionFromClient, const google::protobuf::Message* pMessage)
{
	const gate_handshake_request* pRequest = dynamic_cast<const gate_handshake_request*>(pMessage);
	DebugAst(pRequest != nullptr);

	const std::string& szKey = pRequest->key();
	base::CTokenParser sTokenParser;
	if (!sTokenParser.parse(szKey.c_str(), '|') || sTokenParser.getElementCount() != 2)
	{
		PrintWarning("CClientMessageHandler::handshake error token parse error socket_id: "UINT64FMT" key: %s", pConnectionFromClient->getID(), szKey.c_str());
		pConnectionFromClient->shutdown(true, "token error");
		return;
	}

	uint64_t nPlayerID = 0;
	if (!sTokenParser.getUint64Element(0, nPlayerID))
	{
		PrintWarning("CClientMessageHandler::handshake error parse player_id error socket_id: "UINT64FMT" key: %s", pConnectionFromClient->getID(), szKey.c_str());
		pConnectionFromClient->shutdown(true, "token error");
		return;
	}

	char szTokenBuf[256] = { 0 };
	if (!sTokenParser.getStringElement(1, szTokenBuf, _countof(szTokenBuf)))
	{
		PrintWarning("CClientMessageHandler::handshake error parse token error socket_id: "UINT64FMT" key: %s", pConnectionFromClient->getID(), szKey.c_str());
		pConnectionFromClient->shutdown(true, "token error");
		return;
	}

	const std::vector<uint32_t>& vecServiceID = CBaseApp::Inst()->getServiceIDByTypeName("gas");
	if (vecServiceID.empty())
	{
		PrintWarning("CClientMessageHandler::handshake error gas service not find socket_id: "UINT64FMT" key: %s", pConnectionFromClient->getID(), szKey.c_str());
		pConnectionFromClient->shutdown(true, "token error");
		return;
	}

	uint32_t nIndex = base::CRandGen::getGlobalRand((uint32_t)vecServiceID.size());
	uint32_t nServiceID = vecServiceID[nIndex];

	CClientSession* pClientSession = this->m_pGateService->getClientSessionMgr()->createSession(pConnectionFromClient->getID(), nServiceID, nPlayerID, szTokenBuf, pConnectionFromClient);
	if (nullptr == pClientSession)
	{
		PrintWarning("CClientMessageHandler::handshake error create session error socket_id: "UINT64FMT" key: %s", pConnectionFromClient->getID(), szKey.c_str());
		pConnectionFromClient->shutdown(true, "token error");
		return;
	}

	PrintInfo("CClientMessageHandler::handshake ok wait player enter response service_id: %d player_id: "UINT64FMT" session_id: "UINT64FMT" socket_id: "UINT64FMT, pClientSession->getServiceID(), pClientSession->getPlayerID(), pClientSession->getSessionID(), pClientSession->getSocketID());

	player_enter_request request_msg;
	request_msg.set_player_id(nPlayerID);
	bool bRet = this->m_pGateService->getServiceInvoker()->async_call<player_enter_response>("gas", eSST_Random, 0, &request_msg, [this, nPlayerID](const player_enter_response* pResponse, uint32_t nErrorCode)
	{
		CClientSession* pClientSession = this->m_pGateService->getClientSessionMgr()->getSessionByPlayerID(nPlayerID);
		if (nullptr == pClientSession)
		{
			PrintInfo("CClientMessageHandler::handshake not find session player_id: "UINT64FMT, nPlayerID);
			return;
		}

		if (nErrorCode != eRRT_OK || pResponse == nullptr)
		{
			PrintWarning("CClientMessageHandler::handshake error enter player error player_id: "UINT64FMT, nPlayerID);

			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pClientSession->getSocketID());
			if (nullptr != pBaseConnection)
			{
				pBaseConnection->shutdown(true, "wait player enter error");
			}

			return;
		}

		gate_handshake_response response_msg;
		response_msg.set_player_id(nPlayerID);
		response_msg.set_result(pResponse->result());
		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pClientSession->getSocketID());
		DebugAst(pBaseConnection != nullptr);

		if (pResponse->result() != 0)
		{
			PrintInfo("CClientMessageHandler::handshake result error result: %d service_id: %d player_id: "UINT64FMT" session_id: "UINT64FMT" socket_id: "UINT64FMT, pClientSession->getServiceID(), pResponse->result(), pClientSession->getPlayerID(), pClientSession->getSessionID(), pClientSession->getSocketID());
			
			this->sendClientMessage(pBaseConnection, &response_msg);
			pBaseConnection->shutdown(false, "wait player enter error");

			return;
		}
		pClientSession->setState(eCSS_Normal);

		this->sendClientMessage(pBaseConnection, &response_msg);
		PrintInfo("CClientMessageHandler::handshake ok service_id: %d player_id: "UINT64FMT" session_id: "UINT64FMT" socket_id: "UINT64FMT, pClientSession->getServiceID(), pClientSession->getPlayerID(), pClientSession->getSessionID(), pClientSession->getSocketID());
	});

	if (!bRet)
	{
		PrintWarning("CClientMessageHandler::handshake call enter player error player_id: "UINT64FMT, nPlayerID);
		pConnectionFromClient->shutdown(true, "call enter player error");
	}
}