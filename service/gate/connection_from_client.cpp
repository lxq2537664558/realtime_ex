#include "stdafx.h"
#include "connection_from_client.h"
#include "gate_message_dispatcher.h"
#include "gate_app.h"

#include "libCoreServiceKit/cluster_invoker.h"

static int32_t client_message_parser(const char* pData, uint32_t nSize, uint8_t& nMessageType)
{
	if (nSize < sizeof(core::client_message_header))
		return 0;

	const core::client_message_header* pHeader = reinterpret_cast<const core::client_message_header*>(pData);
	if (pHeader->nMessageSize < sizeof(core::client_message_header))
		return -1;

	// 不是完整的消息
	if (nSize < pHeader->nMessageSize)
		return 0;

	nMessageType = eMT_CLIENT;

	return pHeader->nMessageSize;
}

CConnectionFromClient::CConnectionFromClient()
{

}

CConnectionFromClient::~CConnectionFromClient()
{

}

bool CConnectionFromClient::init(const std::string& szContext)
{
	this->setMessageParser(std::bind(&client_message_parser, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return true;
}

uint32_t CConnectionFromClient::getType() const
{
	return eBCT_ConnectionFromClient;
}

void CConnectionFromClient::release()
{
	delete this;
}

void CConnectionFromClient::onConnect()
{

}

void CConnectionFromClient::onDisconnect()
{
	CGateApp::Inst()->getGateSessionMgr()->delSessionbySocketID(this->getID());
}

void CConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	CGateMessageDispatcher::Inst()->dispatch(this->getID(), nMessageType, pData, nSize);
}