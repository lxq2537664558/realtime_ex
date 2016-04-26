#include "stdafx.h"
#include "gate_session_mgr.h"

CGateSessionMgr::CGateSessionMgr()
{

}

CGateSessionMgr::~CGateSessionMgr()
{

}

bool CGateSessionMgr::init()
{
	return true;
}

CGateSession* CGateSessionMgr::createSession(uint64_t nSocketID, uint64_t nSessionID)
{
	if (this->getSessionBySessionID(nSessionID) != nullptr)
		return nullptr;

	CGateSession* pGateSession = new CGateSession();
	if (!pGateSession->init(nSocketID, nSessionID))
	{
		SAFE_DELETE(pGateSession);
		return nullptr;
	}
	this->m_mapSessionIDBySocketID[nSocketID] = nSessionID;
	this->m_mapGateSessionBySessionID[nSessionID] = pGateSession;

	PrintInfo("create session session_id: "UINT64FMT" socket_id: "UINT64FMT, nSessionID, nSocketID);

	return pGateSession;
}

CGateSession* CGateSessionMgr::getSessionBySessionID(uint64_t nSessionID) const
{
	auto iter = this->m_mapGateSessionBySessionID.find(nSessionID);
	if (iter == this->m_mapGateSessionBySessionID.end())
		return nullptr;

	return iter->second;
}

void CGateSessionMgr::delSessionbySocketID(uint64_t nSocketID)
{
	auto iter = this->m_mapSessionIDBySocketID.find(nSocketID);
	if (iter == this->m_mapSessionIDBySocketID.end())
		return;

	uint64_t nSessionID = iter->second;
	this->m_mapSessionIDBySocketID.erase(iter);

	auto iterSession = this->m_mapGateSessionBySessionID.find(nSessionID);
	DebugAst(iterSession != this->m_mapGateSessionBySessionID.end());
	
	CGateSession* pGateSession = iterSession->second;
	SAFE_DELETE(pGateSession);

	this->m_mapGateSessionBySessionID.erase(iterSession);

	PrintInfo("destroy session session_id: "UINT64FMT" socket_id: "UINT64FMT, nSessionID, nSocketID);
}

CGateSession* CGateSessionMgr::getSessionBySocketID(uint64_t nSocketID) const
{
	auto iter = this->m_mapSessionIDBySocketID.find(nSocketID);
	if (iter == this->m_mapSessionIDBySocketID.end())
		return nullptr;

	return this->getSessionBySessionID(iter->second);
}