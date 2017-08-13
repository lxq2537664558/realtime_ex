#include "stdafx.h"
#include "gate_client_session.h"
#include "gate_service.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"

#include "proto_src/player_enter_gas_request.pb.h"
#include "proto_src/player_enter_gas_response.pb.h"
#include "proto_src/gate_handshake_response.pb.h"

using namespace core;

CGateClientSession::CGateClientSession(CGateService* pGateService)
	: CServiceInvokeHolder(pGateService)
	, m_nSessionID(0)
	, m_nSocketID(0)
	, m_nServiceID(0)
	, m_nPlayerID(0)
	, m_nState(eCSS_None)
{
	this->m_tickerTokenTimeout.setCallback(std::bind(&CGateClientSession::onTokenTimeout, this, std::placeholders::_1));
}

CGateClientSession::~CGateClientSession()
{

}

bool CGateClientSession::init(uint64_t nPlayerID, uint64_t nSessionID, const std::string& szToken)
{
	this->m_nSessionID = nSessionID;
	this->m_nPlayerID = nPlayerID;
	this->m_szToken = szToken;
	this->m_nState = eCSS_None;

	this->getGateService()->registerTicker(&this->m_tickerTokenTimeout, 10000, 0, 0);

	return true;
}

uint64_t CGateClientSession::getPlayerID() const
{
	return this->m_nPlayerID;
}

uint32_t CGateClientSession::getServiceID() const
{
	return this->m_nServiceID;
}

void CGateClientSession::setServiceID(uint32_t nServiceID)
{
	this->m_nServiceID = nServiceID;
}

uint32_t CGateClientSession::getState() const
{
	return this->m_nState;
}

void CGateClientSession::setState(EClientSessionState eState)
{
	this->m_nState |= eState;
}

void CGateClientSession::enterGas()
{
	DebugAst(this->m_nState == (eCSS_ClientEnter | eCSS_TokenEnter));
	
	PrintInfo("CClientSession::enterGas player_id: "UINT64FMT, this->m_nPlayerID);
	
	if (this->m_tickerTokenTimeout.isRegister())
		this->getGateService()->unregisterTicker(&this->m_tickerTokenTimeout);

	player_enter_gas_request request_msg;
	request_msg.set_player_id(this->getPlayerID());
	this->async_call<player_enter_gas_response>(eMTT_Service, this->getServiceID(), &request_msg, [this](const player_enter_gas_response* pResponse, uint32_t nErrorCode)
	{
		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(this->m_nSocketID);
		DebugAst(pBaseConnection != nullptr);

		if (nErrorCode != eRRT_OK || pResponse == nullptr)
		{
			PrintWarning("CClientSession::enterGas error enter player error player_id: "UINT64FMT, this->m_nPlayerID);

			this->getGateService()->getGateClientSessionMgr()->delSessionByPlayerID(this->m_nPlayerID);

			return;
		}

		gate_handshake_response response_msg;
		response_msg.set_player_id(this->m_nPlayerID);
		response_msg.set_result(pResponse->result());

		if (pResponse->result() != 0)
		{
			PrintInfo("CClientSession::enterGas result error result: %d service_id: %d player_id: "UINT64FMT" session_id: "UINT64FMT" socket_id: "UINT64FMT, pResponse->result(), this->m_nServiceID, this->m_nPlayerID, this->m_nSessionID, this->m_nSocketID);

			this->getGateService()->getGateClientMessageHandler()->sendClientMessage(pBaseConnection, &response_msg);
			pBaseConnection->shutdown(false, "player enter gas error");

			return;
		}

		this->setState(eCSS_Normal);

		this->getGateService()->getGateClientMessageHandler()->sendClientMessage(pBaseConnection, &response_msg);
		PrintInfo("CClientSession::enterGas result ok service_id: %d player_id: "UINT64FMT" session_id: "UINT64FMT" socket_id: "UINT64FMT, this->m_nServiceID, this->m_nPlayerID, this->m_nSessionID, this->m_nSocketID);
	});
}

void CGateClientSession::onTokenTimeout(uint64_t nContext)
{
	PrintInfo("CGateClientSession::onTokenTimeout player_id: "UINT64FMT, this->m_nPlayerID);
	
	this->getGateService()->getGateClientSessionMgr()->delSessionByPlayerID(this->m_nPlayerID);
	// 这个时候session已经删除了
}

uint64_t CGateClientSession::getSessionID() const
{
	return this->m_nSessionID;
}

uint64_t CGateClientSession::getSocketID() const
{
	return this->m_nSocketID;
}

void CGateClientSession::setSocketID(uint64_t nSocketID)
{
	this->m_nSocketID = nSocketID;
}

const std::string& CGateClientSession::getToken() const
{
	return this->m_szToken;
}

void CGateClientSession::setToken(const std::string& szToken)
{
	this->m_szToken = szToken;
}

CGateService* CGateClientSession::getGateService() const
{
	return dynamic_cast<CGateService*>(this->getServiceBase());
}
