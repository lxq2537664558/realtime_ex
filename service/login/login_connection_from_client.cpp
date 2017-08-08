#include "stdafx.h"
#include "login_connection_from_client.h"
#include "client_message_dispatcher.h"
#include "login_service.h"

#include "libCoreCommon/base_app.h"

using namespace core;

CLoginConnectionFromClient::CLoginConnectionFromClient()
	: m_pLoginService(nullptr)
	, m_nAccountID(0)
{

}

CLoginConnectionFromClient::~CLoginConnectionFromClient()
{

}

void CLoginConnectionFromClient::release()
{
	delete this;
}

void CLoginConnectionFromClient::onConnect()
{
	uint32_t nServiceID = 0;
	base::crt::atoui(this->getContext().c_str(), nServiceID);
	this->m_pLoginService = dynamic_cast<CLoginService*>(CBaseApp::Inst()->getServiceBase(nServiceID));
	if (nullptr == this->m_pLoginService)
	{
		PrintWarning("login service id error service_id: %d", nServiceID);
		this->shutdown(true, "nullptr == this->m_pGateService");
		return;
	}

	PrintInfo("CLoginConnectionFromClient::onConnect socket_id: "UINT64FMT, this->getID());
}

void CLoginConnectionFromClient::onDisconnect()
{
	PrintInfo("CLoginConnectionFromClient::onDisconnect account_id: "UINT64FMT" socket_id: "UINT64FMT, this->m_nAccountID, this->getID());
}

void CLoginConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);

	this->m_pLoginService->getClientMessageDispatcher()->dispatch(this, pData, nSize);
}

void CLoginConnectionFromClient::setAccountID(uint64_t nAccountID)
{
	this->m_nAccountID = nAccountID;
}
