#include "stdafx.h"
#include "connection_from_client.h"
#include "gate_message_dispatcher.h"
#include "gate_app.h"

#include "libCoreServiceKit/cluster_invoker.h"

CConnectionFromClient::CConnectionFromClient()
{

}

CConnectionFromClient::~CConnectionFromClient()
{

}

bool CConnectionFromClient::init(const std::string& szContext)
{
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
	this->setMessageParser(default_client_message_parser);
	CGateApp::Inst()->getGateSessionMgr()->createSession(this->getID(), this->getID());
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