#pragma once

#include "client_session.h"

#include <map>

class CGateService;
class CGateConnectionFromClient;
class CClientSessionMgr
{
public:
	CClientSessionMgr(CGateService* pGateService);
	~CClientSessionMgr();

	CClientSession*	createSession(uint64_t nPlayerID, const std::string& szToken);
	CClientSession*	getSessionByPlayerID(uint64_t nPlayerID) const;
	CClientSession*	getSessionBySocketID(uint64_t nSocketID) const;
	void			delSessionByPlayerID(uint64_t nPlayerID);

	void			bindSocketID(uint64_t nPlayerID, uint64_t nSocketID);
	void			unbindSocketID(uint64_t nPlayerID);

	uint32_t		getSessionCount() const;

private:
	uint32_t							m_nNextSessionID;
	std::map<uint64_t, CClientSession*>	m_mapClientSessionByPlayerID;
	std::map<uint64_t, uint64_t>		m_mapPlayerIDBySocketID;
	CGateService*						m_pGateService;
};