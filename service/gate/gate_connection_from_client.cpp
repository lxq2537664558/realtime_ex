#include "stdafx.h"
#include "gate_connection_from_client.h"
#include "gate_client_message_dispatcher.h"
#include "gate_client_session_mgr.h"
#include "gate_service.h"

#include "libCoreCommon/base_app.h"

using namespace core;

CGateConnectionFromClient::CGateConnectionFromClient()
	: m_pGateService(nullptr)
{

}

CGateConnectionFromClient::~CGateConnectionFromClient()
{

}

void CGateConnectionFromClient::release()
{
	delete this;
}

void CGateConnectionFromClient::onConnect()
{
	uint32_t nServiceID = 0;
	base::crt::atoui(this->getContext().c_str(), nServiceID);
	this->m_pGateService = dynamic_cast<CGateService*>(CBaseApp::Inst()->getServiceBase(nServiceID));
	if (nullptr == this->m_pGateService)
	{
		PrintWarning("gate service id error service_id: %d", nServiceID);
		this->shutdown(true, "nullptr == this->m_pGateService");
		return;
	}
}

void CGateConnectionFromClient::onDisconnect()
{
	if (this->m_pGateService != nullptr)
	{
		CGateClientSession* pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->getSessionBySocketID(this->getID());
		if (pGateClientSession != nullptr)
		{
			this->m_pGateService->getGateClientSessionMgr()->unbindSocketID(pGateClientSession->getPlayerID());
			this->m_pGateService->getGateClientSessionMgr()->delSessionByPlayerID(pGateClientSession->getPlayerID());
		}
	}
}

void CGateConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	this->m_pGateService->getGateClientMessageDispatcher()->dispatch(this, pData, nSize);
}