#include "gate_connection_from_client.h"
#include "gate_client_message_dispatcher.h"
#include "gate_client_session_mgr.h"
#include "gate_service.h"

#include "libCoreCommon/base_app.h"
#include "libBaseCommon/string_util.h"

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
	base::string_util::convert_to_value(this->getContext(), nServiceID);
	this->m_pGateService = dynamic_cast<CGateService*>(CBaseApp::Inst()->getServiceBase(nServiceID));
	if (nullptr == this->m_pGateService)
	{
		PrintWarning("gate service id error service_id: {}", nServiceID);
		this->shutdown(true, "nullptr == this->m_pGateService");
		return;
	}
}

void CGateConnectionFromClient::onDisconnect()
{
	if (this->m_pGateService != nullptr)
	{
		CGateClientSession* pGateClientSession = this->m_pGateService->getGateClientSessionMgr()->getSessionBySocketID(this->getID());
		if (pGateClientSession != nullptr && pGateClientSession->getSocketID() == this->getID())
		{
			this->m_pGateService->getGateClientSessionMgr()->unbindSocketID(pGateClientSession->getPlayerID());
			this->m_pGateService->getGateClientSessionMgr()->destroySession(pGateClientSession->getPlayerID(), "client disconnect");
		}
	}
}

void CGateConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	this->m_pGateService->getGateClientMessageDispatcher()->dispatch(this, pData, nSize);
}