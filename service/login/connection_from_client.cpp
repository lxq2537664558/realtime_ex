#include "stdafx.h"
#include "connection_from_client.h"
#include "client_message_dispatcher.h"
#include "client_session_mgr.h"
#include "gate_service.h"

#include "libCoreCommon\base_app.h"

using namespace core;

CConnectionFromClient::CConnectionFromClient()
	: m_pGateService(nullptr)
{

}

CConnectionFromClient::~CConnectionFromClient()
{

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
	uint32_t nServiceID = 0;
	base::crt::atoui(this->getContext(), nServiceID);
	this->m_pGateService = dynamic_cast<CGateService*>(CBaseApp::Inst()->getServiceBase(nServiceID));
	if (nullptr == this->m_pGateService)
	{
		PrintWarning("gate service id error service_id: %d", nServiceID);
		this->shutdown(true, "nullptr == this->m_pGateService");
		return;
	}
}

void CConnectionFromClient::onDisconnect()
{
	if(this->m_pGateService != nullptr)
		this->m_pGateService->getClientSessionMgr()->delSessionbySocketID(this->getID());
}

void CConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	this->m_pGateService->getClientMessageDispatcher()->dispatch(this, pData, nSize);
}