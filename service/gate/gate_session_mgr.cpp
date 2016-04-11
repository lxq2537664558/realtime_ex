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

CGateSession* CGateSessionMgr::createSession(uint64_t nSessionID)
{
	if (this->getSession(nSessionID) != nullptr)
		return nullptr;

	CGateSession* pGateSession = new CGateSession(nSessionID);

	this->m_mapGateSession[nSessionID] = pGateSession;

	return pGateSession;
}

CGateSession* CGateSessionMgr::getSession(uint64_t nSessionID) const
{
	auto iter = this->m_mapGateSession.find(nSessionID);
	if (iter == this->m_mapGateSession.end())
		return nullptr;

	return iter->second;
}

void CGateSessionMgr::delSession(uint64_t nSessionID)
{
	auto iter = this->m_mapGateSession.find(nSessionID);
	if (iter == this->m_mapGateSession.end())
		return;

	CGateSession* pGateSession = iter->second;
	SAFE_DELETE(pGateSession);

	this->m_mapGateSession.erase(iter);
}