#include "stdafx.h"
#include "connection_from_client.h"
#include "gate_message_dispatcher.h"
#include "gate_app.h"

#include "libCoreServiceKit/cluster_invoker.h"

DEFINE_OBJECT(CConnectionFromClient, 100)

CConnectionFromClient::CConnectionFromClient()
{

}

CConnectionFromClient::~CConnectionFromClient()
{

}

void CConnectionFromClient::onConnect(const std::string& szContext)
{

}

void CConnectionFromClient::onDisconnect()
{
	CGateApp::Inst()->getGateSessionMgr()->delSessionbySocketID(this->getID());
}

void CConnectionFromClient::onDispatch(uint32_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	CGateMessageDispatcher::Inst()->dispatch(this->getID(), nMessageType, pData, nSize);
}