#pragma once

#include "client_session.h"

#include <map>

class CGateService;
class CConnectionFromClient;
class CClientSessionMgr
{
public:
	CClientSessionMgr(CGateService* pGateService);
	~CClientSessionMgr();

	CClientSession*	createSession(uint64_t nSocketID, uint32_t nServiceID, uint64_t nPlayerID, const std::string& szToken, CConnectionFromClient* pConnectionFromClient);
	CClientSession*	getSessionByPlayerID(uint64_t nActorID) const;
	CClientSession*	getSessionBySocketID(uint64_t nSocketID) const;
	void			delSessionbySocketID(uint64_t nSocketID);

private:
	uint32_t							m_nNextSessionID;
	std::map<uint64_t, CClientSession*>	m_mapClientSessionByPlayerID;
	std::map<uint64_t, uint64_t>		m_mapPlayerIDBySocketID;
	CGateService*						m_pGateService;
};