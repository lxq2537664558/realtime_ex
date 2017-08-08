#include "stdafx.h"
#include "gate_connection_from_client.h"
#include "client_message_dispatcher.h"
#include "client_session_mgr.h"
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
		CClientSession* pClientSession = this->m_pGateService->getClientSessionMgr()->getSessionBySocketID(this->getID());
		if (pClientSession != nullptr)
		{
			this->m_pGateService->getClientSessionMgr()->unbindSocketID(pClientSession->getPlayerID());
			this->m_pGateService->getClientSessionMgr()->delSessionByPlayerID(pClientSession->getPlayerID());
		}
	}
}

void CGateConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	this->m_pGateService->getClientMessageDispatcher()->dispatch(this, pData, nSize);
}