#include "gate_client_session.h"
#include "gate_service.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"

#include "msg_proto_src/g2s_player_enter_request.pb.h"
#include "msg_proto_src/g2s_player_enter_response.pb.h"
#include "msg_proto_src/gate_handshake_response.pb.h"
#include "msg_proto_src/g2s_player_heartbeat_notify.pb.h"

using namespace core;

CGateClientSession::CGateClientSession(CGateService* pGateService)
	: CServiceInvokeHolder(pGateService)
	, m_nSessionID(0)
	, m_nSocketID(0)
	, m_nGasID(0)
	, m_nPlayerID(0)
	, m_nState(eCSS_None)
	, m_bKick(false)
{
	this->m_tickerTokenTimeout.setCallback(std::bind(&CGateClientSession::onTokenTimeout, this, std::placeholders::_1));
	this->m_tickerHeartbeat.setCallback(std::bind(&CGateClientSession::onHeartbeat, this, std::placeholders::_1));
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
	
	this->getGateService()->registerTicker(&this->m_tickerHeartbeat, 5000, 5000, 0);

	return true;
}

uint64_t CGateClientSession::getPlayerID() const
{
	return this->m_nPlayerID;
}

uint32_t CGateClientSession::getGasID() const
{
	return this->m_nGasID;
}

void CGateClientSession::setGasID(uint32_t nGasID)
{
	this->m_nGasID = nGasID;
}

uint32_t CGateClientSession::getState() const
{
	return this->m_nState;
}

void CGateClientSession::setState(uint32_t nState)
{
	this->m_nState = nState;
}

void CGateClientSession::enterGas()
{
	DebugAst(this->m_nState == (eCSS_ClientEnter | eCSS_TokenEnter));
	
	PrintInfo("CClientSession::enterGas player_id: {}", this->m_nPlayerID);
	
	if (this->m_tickerTokenTimeout.isRegister())
		this->getGateService()->unregisterTicker(&this->m_tickerTokenTimeout);

	g2s_player_enter_request request_msg;
	request_msg.set_player_id(this->getPlayerID());
	std::shared_ptr<const g2s_player_enter_response> pResponseMessage = nullptr;
	this->sync_invoke(this->getGasID(), &request_msg, pResponseMessage);
	
	CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(this->m_nSocketID);
	DebugAst(pBaseConnection != nullptr);

	if (pResponseMessage == nullptr)
	{
		PrintWarning("CClientSession::enterGas error enter player error player_id: {}", this->m_nPlayerID);

		this->getGateService()->getGateClientSessionMgr()->destroySession(this->m_nPlayerID, "enter gas error");

		return;
	}

	gate_handshake_response response_msg;
	response_msg.set_player_id(this->m_nPlayerID);
	response_msg.set_result(pResponseMessage->result());

	if (pResponseMessage->result() != 0)
	{
		PrintInfo("CClientSession::enterGas result error result: {} service_id: {} player_id: {} session_id: {} socket_id: {}", pResponseMessage->result(), this->m_nGasID, this->m_nPlayerID, this->m_nSessionID, this->m_nSocketID);

		this->getGateService()->getGateClientMessageHandler()->sendClientMessage(pBaseConnection, &response_msg);
		pBaseConnection->shutdown(false, "player enter gas error");

		return;
	}

	this->setState(eCSS_Normal);

	this->getGateService()->getGateClientMessageHandler()->sendClientMessage(pBaseConnection, &response_msg);
	PrintInfo("CClientSession::enterGas result ok service_id: {} player_id: {} gas_id: {} socket_id: {}", this->m_nGasID, this->m_nPlayerID, this->m_nGasID, this->m_nSocketID);
}

void CGateClientSession::onTokenTimeout(uint64_t nContext)
{
	PrintInfo("CGateClientSession::onTokenTimeout player_id: {}", this->m_nPlayerID);
	
	this->getGateService()->getGateClientSessionMgr()->destroySession(this->m_nPlayerID, "token timeout");
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

void CGateClientSession::onHeartbeat(uint64_t nContext)
{
	g2s_player_heartbeat_notify notify_msg;
	notify_msg.set_player_id(this->m_nPlayerID);
	
	this->getServiceBase()->getServiceInvoker()->send(this->m_nGasID, &notify_msg);
}

void CGateClientSession::setKick(bool bKick)
{
	this->m_bKick = bKick;
}

bool CGateClientSession::isKick() const
{
	return this->m_bKick;
}