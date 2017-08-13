#include "stdafx.h"
#include "gate_client_session_mgr.h"
#include "gate_service.h"
#include "gate_connection_from_client.h"

#include "libBaseCommon/rand_gen.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/base_app.h"

#include "proto_src/player_leave_gas_notify.pb.h"
#include "libCoreCommon/base_connection_mgr.h"

using namespace core;

CGateClientSessionMgr::CGateClientSessionMgr(CGateService* pGateService)
	: m_pGateService(pGateService)
	, m_nNextSessionID(1)
{

}

CGateClientSessionMgr::~CGateClientSessionMgr()
{

}

CGateClientSession* CGateClientSessionMgr::createSession(uint64_t nPlayerID, const std::string& szToken)
{
	if (this->getSessionByPlayerID(nPlayerID) != nullptr)
	{
		PrintWarning("CClientSessionMgr::createSession error player_id: "UINT64FMT, nPlayerID);
		return nullptr;
	}

	uint64_t nSessionID = (uint64_t)base::CRandGen::getGlobalRand(1000000000) << 32 | this->m_nNextSessionID++;

	CGateClientSession* pGateClientSession = new CGateClientSession(this->m_pGateService);
	if (!pGateClientSession->init(nPlayerID, nSessionID, szToken))
	{
		SAFE_DELETE(pGateClientSession);
		return nullptr;
	}
	this->m_mapClientSessionByPlayerID[nPlayerID] = pGateClientSession;

	PrintInfo("create session player_id: "UINT64FMT" session_id: "UINT64FMT, nPlayerID, nSessionID);

	return pGateClientSession;
}

CGateClientSession* CGateClientSessionMgr::getSessionByPlayerID(uint64_t nPlayerID) const
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	if (iter == this->m_mapClientSessionByPlayerID.end())
		return nullptr;

	return iter->second;
}

void CGateClientSessionMgr::delSessionByPlayerID(uint64_t nPlayerID)
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	if (iter == this->m_mapClientSessionByPlayerID.end())
		return;

	this->m_mapClientSessionByPlayerID.erase(iter);

	CGateClientSession* pGateClientSession = iter->second;
	DebugAst(pGateClientSession != nullptr);
	
	this->m_mapPlayerIDBySocketID.erase(pGateClientSession->getSocketID());

	uint64_t nSocketID = pGateClientSession->getSocketID();
	if (nSocketID != 0)
	{
		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(nSocketID);
		if (nullptr != pBaseConnection)
			pBaseConnection->shutdown(true, "kick");
	}

	uint32_t nServiceID = pGateClientSession->getServiceID();
	SAFE_DELETE(pGateClientSession);

	PrintInfo("destroy session player_id: "UINT64FMT" socket_id: "UINT64FMT, nPlayerID, nSocketID);

	player_leave_gas_notify msg;
	msg.set_player_id(nPlayerID);
	this->m_pGateService->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg);
}

void CGateClientSessionMgr::bindSocketID(uint64_t nPlayerID, uint64_t nSocketID)
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	DebugAst(iter != this->m_mapClientSessionByPlayerID.end());

	CGateClientSession* pGateClientSession = iter->second;
	DebugAst(pGateClientSession != nullptr);

	this->m_mapPlayerIDBySocketID[nSocketID] = nPlayerID;
	pGateClientSession->setSocketID(nSocketID);

	PrintInfo("bind socket id player_id: "UINT64FMT" socket_id: "UINT64FMT, nPlayerID, nSocketID);
}

void CGateClientSessionMgr::unbindSocketID(uint64_t nPlayerID)
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	DebugAst(iter != this->m_mapClientSessionByPlayerID.end());

	CGateClientSession* pGateClientSession = iter->second;
	DebugAst(pGateClientSession != nullptr);

	this->m_mapPlayerIDBySocketID.erase(pGateClientSession->getSocketID());
	pGateClientSession->setSocketID(0);

	PrintInfo("unbind socket id player_id: "UINT64FMT, nPlayerID);
}

uint32_t CGateClientSessionMgr::getSessionCount() const
{
	return (uint32_t)this->m_mapClientSessionByPlayerID.size();
}

CGateClientSession* CGateClientSessionMgr::getSessionBySocketID(uint64_t nSocketID) const
{
	auto iter = this->m_mapPlayerIDBySocketID.find(nSocketID);
	if (iter == this->m_mapPlayerIDBySocketID.end())
		return nullptr;

	return this->getSessionByPlayerID(iter->second);
}