#include "login_connection_from_client.h"
#include "login_client_message_dispatcher.h"
#include "login_service.h"

#include "libCoreCommon/base_app.h"
#include "libBaseCommon/string_util.h"

using namespace core;

CLoginConnectionFromClient::CLoginConnectionFromClient()
	: m_pLoginService(nullptr)
	, m_nServerID(0)
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
	base::string_util::convert_to_value(this->getContext(), nServiceID);
	this->m_pLoginService = dynamic_cast<CLoginService*>(CBaseApp::Inst()->getServiceBase(nServiceID));
	if (nullptr == this->m_pLoginService)
	{
		PrintWarning("login service id error service_id: {}", nServiceID);
		this->shutdown(true, "nullptr == this->m_pGateService");
		return;
	}

	PrintInfo("CLoginConnectionFromClient::onConnect socket_id: {}", this->getID());
}

void CLoginConnectionFromClient::onDisconnect()
{
	PrintInfo("CLoginConnectionFromClient::onDisconnect account_name: {} server_id: {} socket_id: {}", this->m_szAccountName, this->m_nServerID, this->getID());
}

void CLoginConnectionFromClient::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
{
	DebugAst(nMessageType == eMT_CLIENT);
	this->m_pLoginService->getLoginClientMessageDispatcher()->dispatch(this, pData, nSize);
}

void CLoginConnectionFromClient::setAccountInfo(const std::string& szAccountName, uint32_t nServerID)
{
	this->m_szAccountName = szAccountName;
	this->m_nServerID = nServerID;
}