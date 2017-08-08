#include "stdafx.h"
#include "client_session.h"
#include "gate_service.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/service_invoker.h"

#include "proto_src/player_enter_gas_request.pb.h"
#include "proto_src/player_enter_gas_response.pb.h"
#include "proto_src/gate_handshake_response.pb.h"

using namespace core;

CClientSession::CClientSession()
	: m_nSessionID(0)
	, m_nSocketID(0)
	, m_nServiceID(0)
	, m_nPlayerID(0)
	, m_nState(eCSS_None)
{

}

CClientSession::~CClientSession()
{

}

bool CClientSession::init(uint64_t nPlayerID, uint64_t nSessionID, const std::string& szToken)
{
	this->m_nSessionID = nSessionID;
	this->m_nPlayerID = nPlayerID;
	this->m_szToken = szToken;
	this->m_nState = eCSS_None;

	return true;
}

uint64_t CClientSession::getPlayerID() const
{
	return this->m_nPlayerID;
}

uint32_t CClientSession::getServiceID() const
{
	return this->m_nServiceID;
}

void CClientSession::setServiceID(uint32_t nServiceID)
{
	this->m_nServiceID = nServiceID;
}

uint32_t CClientSession::getState() const
{
	return this->m_nState;
}

void CClientSession::setState(EClientSessionState eState)
{
	this->m_nState |= eState;
}

void CClientSession::enterGas(CGateService* pGateService)
{
	DebugAst(pGateService != nullptr);
	DebugAst(this->m_nState == (eCSS_ClientEnter | eCSS_TokenEnter));
	
	PrintInfo("CClientSession::enterGas player_id: "UINT64FMT, this->m_nPlayerID);
	
	uint64_t nPlayerID = this->m_nPlayerID;

	player_enter_gas_request request_msg;
	request_msg.set_player_id(this->getPlayerID());
	pGateService->getServiceInvoker()->async_call<player_enter_gas_response>(eMTT_Service, this->getServiceID(), &request_msg, [pGateService, nPlayerID](const player_enter_gas_response* pResponse, uint32_t nErrorCode)
	{
		CClientSession* pClientSession = pGateService->getClientSessionMgr()->getSessionByPlayerID(nPlayerID);
		if (nullptr == pClientSession)
		{
			PrintInfo("CClientSession::enterGas not find session player_id: "UINT64FMT, nPlayerID);
			return;
		}

		if (nErrorCode != eRRT_OK || pResponse == nullptr)
		{
			PrintWarning("CClientSession::enterGas error enter player error player_id: "UINT64FMT, nPlayerID);

			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pClientSession->getSocketID());
			if (nullptr != pBaseConnection)
			{
				pBaseConnection->shutdown(true, "nullptr != pBaseConnection");
			}

			return;
		}

		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pClientSession->getSocketID());
		DebugAst(pBaseConnection != nullptr);

		gate_handshake_response response_msg;
		response_msg.set_player_id(nPlayerID);
		response_msg.set_result(pResponse->result());

		if (pResponse->result() != 0)
		{
			PrintInfo("CClientSession::enterGas result error result: %d service_id: %d player_id: "UINT64FMT" session_id: "UINT64FMT" socket_id: "UINT64FMT, pClientSession->getServiceID(), pResponse->result(), pClientSession->getPlayerID(), pClientSession->getSessionID(), pClientSession->getSocketID());

			pGateService->getClientMessageHandler()->sendClientMessage(pBaseConnection, &response_msg);
			pBaseConnection->shutdown(false, "player enter gas error");

			return;
		}
		pClientSession->setState(eCSS_Normal);

		pGateService->getClientMessageHandler()->sendClientMessage(pBaseConnection, &response_msg);
		PrintInfo("CClientSession::enterGas ok service_id: %d player_id: "UINT64FMT" session_id: "UINT64FMT" socket_id: "UINT64FMT, pClientSession->getServiceID(), pClientSession->getPlayerID(), pClientSession->getSessionID(), pClientSession->getSocketID());
	});
}

uint64_t CClientSession::getSessionID() const
{
	return this->m_nSessionID;
}

uint64_t CClientSession::getSocketID() const
{
	return this->m_nSocketID;
}

void CClientSession::setSocketID(uint64_t nSocketID)
{
	this->m_nSocketID = nSocketID;
}

const std::string& CClientSession::getToken() const
{
	return this->m_szToken;
}

void CClientSession::setToken(const std::string& szToken)
{
	this->m_szToken = szToken;
}
