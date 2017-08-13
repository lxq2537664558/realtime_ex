#include "stdafx.h"
#include "gate_client_message_handler.h"
#include "gate_client_message_dispatcher.h"
#include "gate_service.h"

#include "libBaseCommon/token_parser.h"
#include "libBaseCommon/rand_gen.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/service_base.h"

#include "proto_src/gate_handshake_request.pb.h"
#include "proto_src/gate_handshake_response.pb.h"
#include "proto_src/player_enter_gas_request.pb.h"
#include "proto_src/player_enter_gas_response.pb.h"

using namespace core;

CGateClientMessageHandler::CGateClientMessageHandler(CGateService*	pGateService)
	: m_pGateService(pGateService)
{
	this->m_szBuf.resize(UINT16_MAX);

	this->m_pGateService->getGateClientMessageDispatcher()->registerMessageHandler("gate_handshake_request", std::bind(&CGateClientMessageHandler::handshake, this, std::placeholders::_1, std::placeholders::_2));
}

CGateClientMessageHandler::~CGateClientMessageHandler()
{

}

void CGateClientMessageHandler::sendClientMessage(CBaseConnection* pBaseConnection, const google::protobuf::Message* pMessage)
{
	DebugAst(pMessage != nullptr && pBaseConnection != nullptr);

	message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

	int32_t nDataSize = this->m_pGateService->getForwardProtobufFactory()->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + sizeof(message_header), (uint32_t)(this->m_szBuf.size() - sizeof(message_header)));
	if (nDataSize < 0)
		return;

	pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
	pHeader->nMessageID = base::hash(pMessage->GetTypeName().c_str());

	pBaseConnection->send(eMT_CLIENT, &this->m_szBuf[0], pHeader->nMessageSize);
}

void CGateClientMessageHandler::handshake(CGateConnectionFromClient* pGateConnectionFromClient, const google::protobuf::Message* pMessage)
{
	const gate_handshake_request* pRequest = dynamic_cast<const gate_handshake_request*>(pMessage);
	DebugAst(pRequest != nullptr);

	const std::string& szKey = pRequest->key();
	base::CTokenParser sTokenParser;
	if (!sTokenParser.parse(szKey.c_str(), '|') || sTokenParser.getElementCount() != 2)
	{
		PrintWarning("CGateClientMessageHandler::handshake error token parse error socket_id: "UINT64FMT" key: %s", pGateConnectionFromClient->getID(), szKey.c_str());
		pGateConnectionFromClient->shutdown(true, "token error");
		return;
	}

	uint64_t nPlayerID = 0;
	if (!sTokenParser.getUint64Element(0, nPlayerID))
	{
		PrintWarning("CGateClientMessageHandler::handshake error parse player_id error socket_id: "UINT64FMT" key: %s", pGateConnectionFromClient->getID(), szKey.c_str());
		pGateConnectionFromClient->shutdown(true, "token error");
		return;
	}

	char szTokenBuf[256] = { 0 };
	if (!sTokenParser.getStringElement(1, szTokenBuf, _countof(szTokenBuf)))
	{
		PrintWarning("CGateClientMessageHandler::handshake error parse token error socket_id: "UINT64FMT" key: %s", pGateConnectionFromClient->getID(), szKey.c_str());
		pGateConnectionFromClient->shutdown(true, "token error");
		return;
	}

	PrintInfo("CGateClientMessageHandler::handshake player_id: "UINT64FMT, nPlayerID);

	CGateClientSession* pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->getSessionByPlayerID(nPlayerID);
	if (nullptr != pGateClientSession)
	{
		// 顶号
		if ((pGateClientSession->getState()&eCSS_ClientEnter) != 0)
		{
			// 这里先直接踢掉
			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pGateClientSession->getSocketID());
			if (nullptr == pBaseConnection)
			{
				PrintWarning("CGateClientMessageHandler::handshake nullptr == pBaseConnection player_id: "UINT64FMT, pGateClientSession->getPlayerID());
				this->m_pGateService->getGateClientSessionMgr()->delSessionByPlayerID(pGateClientSession->getPlayerID());
				return;
			}
			pBaseConnection->shutdown(true, "dup");
			pGateConnectionFromClient->shutdown(true, "dup");
			return;
		}
		DebugAst(pGateClientSession->getState() == eCSS_TokenEnter);

		if (pGateClientSession->getToken() != szTokenBuf)
		{
			PrintWarning("CGateClientMessageHandler::handshake token error player_id: "UINT64FMT, nPlayerID);

			pGateConnectionFromClient->shutdown(true, "token error");
			return;
		}

		this->m_pGateService->getGateClientSessionMgr()->bindSocketID(nPlayerID, pGateConnectionFromClient->getID());
		pGateClientSession->setState(eCSS_ClientEnter);
		pGateClientSession->enterGas();
	}
	else
	{
		pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->createSession(nPlayerID, szTokenBuf);
		if (nullptr == pGateClientSession)
		{
			PrintWarning("CGateClientMessageHandler::handshake error create session error socket_id: "UINT64FMT" key: %s", pGateConnectionFromClient->getID(), szKey.c_str());
			pGateConnectionFromClient->shutdown(true, "token error");
			return;
		}

		this->m_pGateService->getGateClientSessionMgr()->bindSocketID(nPlayerID, pGateConnectionFromClient->getID());
		pGateClientSession->setState(eCSS_ClientEnter);
	}
}