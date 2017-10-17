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

#include "client_proto_src/c2l_player_login_request.pb.h"
#include "client_proto_src/c2l_player_login_response.pb.h"
#include "server_proto_src/l2d_validate_login_request.pb.h"
#include "server_proto_src/l2d_validate_login_response.pb.h"

#include "../common/error_code.h"

using namespace core;

CLoginClientMessageHandler::CLoginClientMessageHandler(CLoginService*	pLoginService)
	: m_pLoginService(pLoginService)
{
	this->m_szBuf.resize(UINT16_MAX);

	this->m_pLoginService->getLoginClientMessageDispatcher()->registerMessageHandler("c2l_player_login_request", std::bind(&CLoginClientMessageHandler::login, this, std::placeholders::_1, std::placeholders::_2));
}

CLoginClientMessageHandler::~CLoginClientMessageHandler()
{

}

void CLoginClientMessageHandler::sendClientMessage(CBaseConnection* pBaseConnection, const google::protobuf::Message* pMessage)
{
	DebugAst(pMessage != nullptr && pBaseConnection != nullptr);

	message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

	int32_t nDataSize = this->m_pLoginService->getForwardMessageSerializer()->serializeMessageToBuf(pMessage, &this->m_szBuf[0] + sizeof(message_header), (uint32_t)(this->m_szBuf.size() - sizeof(message_header)));
	if (nDataSize < 0)
		return;

	pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
	pHeader->nMessageID = _GET_MESSAGE_ID(pMessage->GetTypeName());

	pBaseConnection->send(eMT_CLIENT, &this->m_szBuf[0], pHeader->nMessageSize);
}

void CLoginClientMessageHandler::login(CLoginConnectionFromClient* pLoginConnectionFromClient, const google::protobuf::Message* pMessage)
{
	const c2l_player_login_request* pRequest = dynamic_cast<const c2l_player_login_request*>(pMessage);
	DebugAst(pRequest != nullptr);

	uint64_t nSocketID = pLoginConnectionFromClient->getID();
	uint32_t nServerID = pRequest->server_id();
	const std::string& szAccountName = pRequest->account_name();

	pLoginConnectionFromClient->setAccountInfo(szAccountName, nServerID);
	PrintInfo("CLoginClientMessageHandler::login account_name: {} server_id: {} socket_id: {}", szAccountName, nServerID, nSocketID);

	l2d_validate_login_request request_msg;
	request_msg.set_account_name(szAccountName);
	request_msg.set_server_id(nServerID);

	std::shared_ptr<const l2d_validate_login_response> pResponseMessage;
	if (this->m_pLoginService->getServiceInvoker()->sync_invoke("dispatch", eSST_Random, 0, &request_msg, pResponseMessage) != eRRT_OK)
	{
		c2l_player_login_response response_msg;
		response_msg.set_result(eEC_Login_InvokeDispatchError);
		response_msg.set_key("");
		response_msg.set_gate_addr("");
		this->sendClientMessage(pLoginConnectionFromClient, &response_msg);

		pLoginConnectionFromClient->shutdown(false, "validate login error");
	}
	
	pLoginConnectionFromClient = dynamic_cast<CLoginConnectionFromClient*>(this->m_pLoginService->getBaseConnectionMgr()->getBaseConnectionBySocketID(nSocketID));
	if (nullptr == pLoginConnectionFromClient)
	{
		PrintInfo("CLoginClientMessageHandler::login nullptr == pLoginConnectionFromClient account_name: {} server_id: {} socket_id: {}", szAccountName, nServerID, nSocketID);
		return;
	}

	if (pResponseMessage == nullptr)
	{
		PrintInfo("CLoginClientMessageHandler::login nErrorCode != eRRT_OK || pResponse == nullptr account_name: {} server_id: {} socket_id: {}", szAccountName, nServerID, nSocketID);

		c2l_player_login_response response_msg;
		response_msg.set_result(eEC_Login_InvokeDispatchError);
		response_msg.set_key("");
		response_msg.set_gate_addr("");
		this->sendClientMessage(pLoginConnectionFromClient, &response_msg);

		pLoginConnectionFromClient->shutdown(false, "validate login error");

		return;
	}

	c2l_player_login_response response_msg;
	response_msg.set_result(pResponseMessage->result());
	response_msg.set_key(pResponseMessage->key());
	response_msg.set_gate_addr(pResponseMessage->gate_addr());

	this->sendClientMessage(pLoginConnectionFromClient, &response_msg);
	PrintInfo("CLoginClientMessageHandler::login ok account_name: {} server_id: {} socket_id: {}", szAccountName, nServerID, nSocketID);
}