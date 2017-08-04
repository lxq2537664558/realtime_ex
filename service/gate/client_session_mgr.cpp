#include "stdafx.h"
#include "client_session_mgr.h"
#include "gate_service.h"
#include "gate_connection_from_client.h"

#include "libBaseCommon\rand_gen.h"
#include "libCoreCommon\core_common.h"

#include "proto_src\player_leave.pb.h"

using namespace core;

CClientSessionMgr::CClientSessionMgr(CGateService* pGateService)
	: m_pGateService(pGateService)
	, m_nNextSessionID(1)
{

}

CClientSessionMgr::~CClientSessionMgr()
{

}

CClientSession* CClientSessionMgr::createSession(uint64_t nSocketID, uint32_t nServiceID, uint64_t nPlayerID, const std::string& szToken, CGateConnectionFromClient* pGateConnectionFromClient)
{
	DebugAstEx(pGateConnectionFromClient != nullptr, nullptr);

	if (this->getSessionByPlayerID(nPlayerID) != nullptr)
	{
		PrintWarning("CClientSessionMgr::createSession error player_id: "UINT64FMT, nPlayerID);
		return nullptr;
	}

	if (this->getSessionBySocketID(nSocketID) != nullptr)
	{
		PrintWarning("CClientSessionMgr::createSession error player_id: "UINT64FMT" socket_id: "UINT64FMT, nPlayerID, nSocketID);
		return nullptr;
	}

	uint64_t nSessionID = (uint64_t)base::CRandGen::getGlobalRand(1000000000) << 32 | this->m_nNextSessionID++;

	CClientSession* pClientSession = new CClientSession();
	if (!pClientSession->init(nSocketID, nSessionID, nServiceID, nPlayerID, szToken))
	{
		SAFE_DELETE(pClientSession);
		return nullptr;
	}
	this->m_mapPlayerIDBySocketID[nSocketID] = nPlayerID;
	this->m_mapClientSessionByPlayerID[nPlayerID] = pClientSession;
	pGateConnectionFromClient->setSessionID(nSessionID);

	PrintInfo("create session player_id: "UINT64FMT"session_id: "UINT64FMT" socket_id: "UINT64FMT, nPlayerID, nSessionID, nSocketID);

	return pClientSession;
}

CClientSession* CClientSessionMgr::getSessionByPlayerID(uint64_t nActorID) const
{
	auto iter = this->m_mapClientSessionByPlayerID.find(nActorID);
	if (iter == this->m_mapClientSessionByPlayerID.end())
		return nullptr;

	return iter->second;
}

void CClientSessionMgr::delSessionbySocketID(uint64_t nSocketID)
{
	auto iter = this->m_mapPlayerIDBySocketID.find(nSocketID);
	if (iter == this->m_mapPlayerIDBySocketID.end())
		return;

	uint64_t nPlayerID = iter->second;
	this->m_mapPlayerIDBySocketID.erase(iter);

	auto iterPlayer = this->m_mapClientSessionByPlayerID.find(nPlayerID);
	DebugAst(iterPlayer != this->m_mapClientSessionByPlayerID.end());
	
	CClientSession* pClientSession = iterPlayer->second;
	uint32_t nServiceID = pClientSession->getServiceID();
	SAFE_DELETE(pClientSession);

	this->m_mapClientSessionByPlayerID.erase(iterPlayer);

	PrintInfo("destroy session player_id: "UINT64FMT" socket_id: "UINT64FMT, nPlayerID, nSocketID);

	player_leave msg;
	msg.set_player_id(nPlayerID);
	this->m_pGateService->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg);
}

CClientSession* CClientSessionMgr::getSessionBySocketID(uint64_t nSocketID) const
{
	auto iter = this->m_mapPlayerIDBySocketID.find(nSocketID);
	if (iter == this->m_mapPlayerIDBySocketID.end())
		return nullptr;

	return this->getSessionByPlayerID(iter->second);
}