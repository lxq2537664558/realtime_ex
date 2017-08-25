#include "connection_to_login.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/function_util.h"
#include "message_handler.h"

#include "msg_proto_src/gate_handshake_request.pb.h"
#include "msg_proto_src/player_login_request.pb.h"
#include <iostream>

CConnectToLogin::CConnectToLogin()
	: m_nAccountID(0)
{
}

CConnectToLogin::~CConnectToLogin()
{

}

void CConnectToLogin::onConnect()
{
	PrintInfo("CConnectToLogin::onConnect");

	std::string szName;
	std::cin >> szName;
	player_login_request request_msg;
	request_msg.set_account_name(szName);
	request_msg.set_server_id(1);

	CMessageHandler::Inst()->sendMessage(this, &request_msg);
}

void CConnectToLogin::onDisconnect()
{
	PrintInfo("CConnectToLogin::onDisconnect");
}

void CConnectToLogin::onDispatch(const message_header* pData)
{
	CMessageHandler::Inst()->dispatch(this, pData);
}

uint32_t CConnectToLogin::onRecv(const char* pData, uint32_t nDataSize)
{
	uint32_t nRecvSize = 0;
	do
	{
		uint16_t nMessageSize = 0;

		// 都不够消息头
		if (nDataSize < sizeof(message_header))
			break;

		const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
		if (pHeader->nMessageSize < sizeof(message_header))
		{
			this->m_pNetConnecter->shutdown(true, "");
			break;
		}

		// 不是完整的消息
		if (nDataSize < pHeader->nMessageSize)
			break;

		nMessageSize = pHeader->nMessageSize;

		this->onDispatch(pHeader);

		nRecvSize += nMessageSize;
		nDataSize -= nMessageSize;
		pData += nMessageSize;

	} while (true);

	return nRecvSize;
}

uint64_t CConnectToLogin::getAccountID() const
{
	return this->m_nAccountID;
}
