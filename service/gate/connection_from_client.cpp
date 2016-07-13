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

}

void CConnectionFromClient::onDisconnect()
{
	CGateApp::Inst()->getGateSessionMgr()->delSessionbySocketID(this->getID());
}

bool CConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAstEx(nMessageType == eMT_CLIENT, true);

	CGateMessageDispatcher::Inst()->dispatch(this->getID(), nMessageType, pData, nSize);

	return true;
}