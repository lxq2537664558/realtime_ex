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
		this->doFoward(pHeader);
}

void CConnectionFromClient::doFoward(const core::message_header* pHeader)
{
	DebugAst(pHeader != nullptr);

	int16_t nMessageSize = pHeader->nMessageSize;
	if (nMessageSize < 0)
	{
		// protobuf消息，去掉在消息长度上打上的标记，换做在消息类型上打上标记
		const_cast<core::message_header*>(pHeader)->nMessageSize = -nMessageSize;
		core::CClusterInvoker::Inst()->foward(this->getID(), eMT_PROTOBUF, pHeader, CGateApp::Inst()->getLoadBalancePolicy(eLBPID_Rand), 0);
	}
	else
	{
		core::CClusterInvoker::Inst()->foward(this->getID(), eMT_NATIVE, pHeader, CGateApp::Inst()->getLoadBalancePolicy(eLBPID_Rand), 0);
	}
}