#include "gate_client_message_handler.h"
#include "gate_client_message_dispatcher.h"
#include "gate_service.h"

#include "libBaseCommon/token_parser.h"
#include "libBaseCommon/rand_gen.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/service_base.h"

#include "msg_proto_src/gate_handshake_request.pb.h"
#include "msg_proto_src/gate_handshake_response.pb.h"

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

	std::string szMessageName = pMessage->GetTypeName();
	pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
	pHeader->nMessageID = _GET_MESSAGE_ID(szMessageName);

	pBaseConnection->send(eMT_CLIENT, &this->m_szBuf[0], pHeader->nMessageSize);
}

void CGateClientMessageHandler::handshake(CGateConnectionFromClient* pGateConnectionFromClient, const google::protobuf::Message* pMessage)
{
	const gate_handshake_request* pRequest = dynamic_cast<const gate_handshake_request*>(pMessage);
	DebugAst(pRequest != nullptr);

	const std::string& szKey = pRequest->key();
	base::CTokenParser sTokenParser;
	if (!sTokenParser.parse(szKey.c_str(), "|") || sTokenParser.getElementCount() != 2)
	{
		PrintWarning("CGateClientMessageHandler::handshake error token parse error socket_id: {} key: {}", pGateConnectionFromClient->getID(), szKey);
		pGateConnectionFromClient->shutdown(true, "token error");
		return;
	}

	uint64_t nPlayerID = 0;
	if (!sTokenParser.getUint64Element(0, nPlayerID))
	{
		PrintWarning("CGateClientMessageHandler::handshake error parse player_id error socket_id: {} key: {}", pGateConnectionFromClient->getID(), szKey);
		pGateConnectionFromClient->shutdown(true, "token error");
		return;
	}

	char szTokenBuf[256] = { 0 };
	if (!sTokenParser.getStringElement(1, szTokenBuf, _countof(szTokenBuf)))
	{
		PrintWarning("CGateClientMessageHandler::handshake error parse token error socket_id: {} key: {}", pGateConnectionFromClient->getID(), szKey);
		pGateConnectionFromClient->shutdown(true, "token error");
		return;
	}

	PrintInfo("CGateClientMessageHandler::handshake 1 player_id: {} token: {}", nPlayerID, szTokenBuf);

	CGateClientSession* pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->getSessionByPlayerID(nPlayerID);
	if (nullptr != pGateClientSession)
	{
		if ((pGateClientSession->getState()&eCSS_TokenEnter) != 0)
		{
			// token不一致，直接踢掉
			if (pGateClientSession->getToken() != szTokenBuf)
			{
				PrintWarning("CGateClientMessageHandler::handshake token error player_id: {} old_token: {} new_token: {}", nPlayerID, pGateClientSession->getToken(), szTokenBuf);

				pGateConnectionFromClient->shutdown(true, "token error");
				return;
			}

			PrintInfo("CGateClientMessageHandler::handshake 2 player_id: {} token: {} gas_id: {}", nPlayerID, pGateClientSession->getToken(), pGateClientSession->getGasID());

			this->m_pGateService->getGateClientSessionMgr()->bindSocketID(nPlayerID, pGateConnectionFromClient->getID());
			pGateClientSession->setState(eCSS_ClientEnter | eCSS_TokenEnter);
			pGateClientSession->enterGas();
			return;
		}

		if (pGateClientSession->getState() != eCSS_Normal)
		{
			PrintInfo("CGateClientMessageHandler::handshake 3 player_id: {} token: {} gas_id: {}", nPlayerID, pGateClientSession->getToken(), pGateClientSession->getGasID());

			pGateConnectionFromClient->shutdown(true, "state error");
			return;
		}
		
		PrintInfo("CGateClientMessageHandler::handshake 4 player_id: {} token: {} gas_id: {}", nPlayerID, pGateClientSession->getToken(), pGateClientSession->getGasID());
		// 顶号
		uint64_t nOldSocketID = pGateClientSession->getSocketID();
		this->m_pGateService->getGateClientSessionMgr()->unbindSocketID(pGateClientSession->getPlayerID());

		// 把老的链接踢掉
		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(nOldSocketID);
		if (nullptr != pBaseConnection)
			pBaseConnection->shutdown(true, "dup");

		pGateClientSession->setState(eCSS_ClientEnter);
		pGateClientSession->setToken(szTokenBuf);
		this->m_pGateService->getGateClientSessionMgr()->bindSocketID(nPlayerID, pGateConnectionFromClient->getID());
	}
	else
	{
		PrintInfo("CGateClientMessageHandler::handshake 5 player_id: {} token: {}", nPlayerID, szTokenBuf);

		pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->createSession(nPlayerID, szTokenBuf);
		if (nullptr == pGateClientSession)
		{
			PrintWarning("CGateClientMessageHandler::handshake error create session error socket_id: {} key: {}", pGateConnectionFromClient->getID(), szKey);
			pGateConnectionFromClient->shutdown(true, "token error");
			return;
		}

		this->m_pGateService->getGateClientSessionMgr()->bindSocketID(nPlayerID, pGateConnectionFromClient->getID());
		pGateClientSession->setState(eCSS_ClientEnter);
	}
}