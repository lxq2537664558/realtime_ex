#include "stdafx.h"
#include "client_session_mgr.h"
#include "gate_service.h"
#include "gate_connection_from_client.h"

#include "libBaseCommon/rand_gen.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/base_app.h"

#include "proto_src/player_leave_gas_notify.pb.h"
#include "libCoreCommon/base_connection_mgr.h"

using namespace core;

CClientSessionMgr::CClientSessionMgr(CGateService* pGateService)
	: m_pGateService(pGateService)
	, m_nNextSessionID(1)
{

}

CClientSessionMgr::~CClientSessionMgr()
{

}

CClientSession* CClientSessionMgr::createSession(uint64_t nPlayerID, const std::string& szToken)
{
	if (this->getSessionByPlayerID(nPlayerID) != nullptr)
	{
		PrintWarning("CClientSessionMgr::createSession error player_id: "UINT64FMT, nPlayerID);
		return nullptr;
	}

	uint64_t nSessionID = (uint64_t)base::CRandGen::getGlobalRand(1000000000) << 32 | this->m_nNextSessionID++;

	CClientSession* pClientSession = new CClientSession();
	if (!pClientSession->init(nPlayerID, nSessionID, szToken))
	{
		SAFE_DELETE(pClientSession);
		return nullptr;
	}
	this->m_mapClientSessionByPlayerID[nPlayerID] = pClientSession;

	PrintInfo("create session player_id: "UINT64FMT" session_id: "UINT64FMT, nPlayerID, nSessionID);

	return pClientSession;
}

CClientSession* CClientSessionMgr::getSessionByPlayerID(uint64_t nPlayerID) const
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	if (iter == this->m_mapClientSessionByPlayerID.end())
		return nullptr;

	return iter->second;
}

void CClientSessionMgr::delSessionByPlayerID(uint64_t nPlayerID)
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	if (iter == this->m_mapClientSessionByPlayerID.end())
		return;

	this->m_mapClientSessionByPlayerID.erase(iter);

	CClientSession* pClientSession = iter->second;
	DebugAst(pClientSession != nullptr);
	
	this->m_mapPlayerIDBySocketID.erase(pClientSession->getSocketID());

	uint64_t nSocketID = pClientSession->getSocketID();
	if (nSocketID != 0)
	{
		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(nSocketID);
		if (nullptr != pBaseConnection)
			pBaseConnection->shutdown(true, "kick");
	}

	uint32_t nServiceID = pClientSession->getServiceID();
	SAFE_DELETE(pClientSession);

	PrintInfo("destroy session player_id: "UINT64FMT" socket_id: "UINT64FMT, nPlayerID, nSocketID);

	player_leave_gas_notify msg;
	msg.set_player_id(nPlayerID);
	this->m_pGateService->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg);
}

void CClientSessionMgr::bindSocketID(uint64_t nPlayerID, uint64_t nSocketID)
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	DebugAst(iter != this->m_mapClientSessionByPlayerID.end());

	CClientSession* pClientSession = iter->second;
	DebugAst(pClientSession != nullptr);

	this->m_mapPlayerIDBySocketID[nSocketID] = nPlayerID;
	pClientSession->setSocketID(nSocketID);

	PrintInfo("bind socket id player_id: "UINT64FMT" socket_id: "UINT64FMT, nPlayerID, nSocketID);
}

void CClientSessionMgr::unbindSocketID(uint64_t nPlayerID)
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	DebugAst(iter != this->m_mapClientSessionByPlayerID.end());

	CClientSession* pClientSession = iter->second;
	DebugAst(pClientSession != nullptr);

	this->m_mapPlayerIDBySocketID.erase(pClientSession->getSocketID());
	pClientSession->setSocketID(0);

	PrintInfo("unbind socket id player_id: "UINT64FMT, nPlayerID);
}

uint32_t CClientSessionMgr::getSessionCount() const
{
	return (uint32_t)this->m_mapClientSessionByPlayerID.size();
}

CClientSession* CClientSessionMgr::getSessionBySocketID(uint64_t nSocketID) const
{
	auto iter = this->m_mapPlayerIDBySocketID.find(nSocketID);
	if (iter == this->m_mapPlayerIDBySocketID.end())
		return nullptr;

	return this->getSessionByPlayerID(iter->second);
}