#include "stdafx.h"
#include "connection_from_client.h"
#include "gate_message_dispatcher.h"
#include "gate_app.h"

#include "libCoreCommon\cluster_invoker.h"

DEFINE_OBJECT(CConnectionFromClient, 100)

CConnectionFromClient::CConnectionFromClient()
	: m_nSessionID(0)
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
	if (this->m_nSessionID != 0)
		CGateApp::Inst()->getGateSessionMgr()->delSession(this->m_nSessionID);
}

void CConnectionFromClient::onDispatch(uint16_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
	if (CGateMessageDispatcher::Inst()->getCallback(pHeader->nMessageID) != nullptr)
		CGateMessageDispatcher::Inst()->dispatch(nMessageType, pData, nSize);
	else
		this->forward(pHeader);
}

void CConnectionFromClient::forward(const core::message_header* pHeader)
{
	DebugAst(pHeader != nullptr);

	core::CClusterInvoker::Inst()->forward(this->getID(), pHeader, CGateApp::Inst()->getLoadBalancePolicy(eLBPID_Rand), 0);
}