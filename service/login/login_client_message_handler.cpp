#include "stdafx.h"
#include "login_client_message_handler.h"
#include "login_client_message_dispatcher.h"
#include "login_service.h"

#include "libBaseCommon/token_parser.h"
#include "libBaseCommon/rand_gen.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"

#include "proto_src/player_login_request.pb.h"
#include "proto_src/validate_login_request.pb.h"
#include "proto_src/validate_login_response.pb.h"
#include "proto_src/player_login_response.pb.h"

using namespace core;

CLoginClientMessageHandler::CLoginClientMessageHandler(CLoginService*	pLoginService)
	: m_pLoginService(pLoginService)
{
	this->m_szBuf.resize(UINT16_MAX);

	this->m_pLoginService->getLoginClientMessageDispatcher()->registerMessageHandler("player_login_request", std::bind(&CLoginClientMessageHandler::login, this, std::placeholders::_1, std::placeholders::_2));
}

CLoginClientMessageHandler::~CLoginClientMessageHandler()
{

}

void CLoginClientMessageHandler::sendClientMessage(CBaseConnection* pBaseConnection, const google::protobuf::Message* pMessage)
{
	DebugAst(pMessage != nullptr && pBaseConnection != nullptr);

	message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

	int32_t nDataSize = this->m_pLoginService->getForwardProtobufFactory()->serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + sizeof(message_header), (uint32_t)(this->m_szBuf.size() - sizeof(message_header)));
	if (nDataSize < 0)
		return;

	pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
	pHeader->nMessageID = base::hash(pMessage->GetTypeName().c_str());

	pBaseConnection->send(eMT_CLIENT, &this->m_szBuf[0], pHeader->nMessageSize);
}

void CLoginClientMessageHandler::login(CLoginConnectionFromClient* pLoginConnectionFromClient, const google::protobuf::Message* pMessage)
{
	const player_login_request* pRequest = dynamic_cast<const player_login_request*>(pMessage);
	DebugAst(pRequest != nullptr);

	uint64_t nSocketID = pLoginConnectionFromClient->getID();
	uint64_t nAccountID = pRequest->account_id();

	pLoginConnectionFromClient->setAccountID(nAccountID);
	PrintInfo("CLoginClientMessageHandler::login account_id: "UINT64FMT" server_id: %d socket_id: "UINT64FMT, nAccountID, pRequest->server_id(), nSocketID);

	validate_login_request request_msg;
	request_msg.set_account_id(pRequest->account_id());
	request_msg.set_server_id(pRequest->server_id());

	this->m_pLoginService->getServiceInvoker()->async_call<validate_login_response>("dispatch", "random", 0, &request_msg, [this, nSocketID, nAccountID](const validate_login_response* pResponse, uint32_t nErrorCode)
	{
		CLoginConnectionFromClient* pLoginConnectionFromClient = dynamic_cast<CLoginConnectionFromClient*>(CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(nSocketID));
		if (nullptr == pLoginConnectionFromClient)
		{
			PrintInfo("CLoginClientMessageHandler::login not find connection account_id: "UINT64FMT" socket_id: "UINT64FMT, nAccountID, nSocketID);
			return;
		}

		if (nErrorCode != eRRT_OK || pResponse == nullptr)
		{
			PrintInfo("CLoginClientMessageHandler::login validate error account_id: "UINT64FMT" socket_id: "UINT64FMT, nAccountID, nSocketID);
			
			player_login_response response_msg;
			response_msg.set_result(1);
			this->sendClientMessage(pLoginConnectionFromClient, &response_msg);

			pLoginConnectionFromClient->shutdown(false, "validate login error");

			return;
		}

		player_login_response response_msg;
		response_msg.set_result(pResponse->result());
		response_msg.set_key(pResponse->key());
		response_msg.set_gate_addr(pResponse->gate_addr());

		this->sendClientMessage(pLoginConnectionFromClient, &response_msg);
		PrintInfo("CLoginClientMessageHandler::login validate ok result: %d account_id: "UINT64FMT" socket_id: "UINT64FMT, pResponse->result(), nAccountID, nSocketID);
	});
}