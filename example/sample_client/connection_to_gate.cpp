#include "stdafx.h"
#include "connection_to_gate.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_function.h"
#include "message_handler.h"

#include "proto_src/gate_handshake_request.pb.h"

CConnectToGate::CConnectToGate(const std::string& szKey)
	: m_nPlayerID(0)
	, m_szKey(szKey)
{
}

CConnectToGate::~CConnectToGate()
{

}

void CConnectToGate::onConnect()
{
	PrintInfo("CConnectToGate::onConnect");

	gate_handshake_request request_msg;
	request_msg.set_key(this->m_szKey);

	CMessageHandler::Inst()->sendMessage(this, &request_msg);
}

void CConnectToGate::onDisconnect()
{
	PrintInfo("CConnectToGate::onDisconnect");
}

void CConnectToGate::onDispatch(const message_header* pData)
{
	CMessageHandler::Inst()->dispatch(this, pData);
}

uint32_t CConnectToGate::onRecv(const char* pData, uint32_t nDataSize)
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

uint64_t CConnectToGate::getPlayerID() const
{
	return this->m_nPlayerID;
}
