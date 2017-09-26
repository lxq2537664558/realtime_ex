#pragma once

#include "gate_client_session.h"

#include <map>

class CGateService;
class CGateConnectionFromClient;
class CGateClientSessionMgr
{
public:
	CGateClientSessionMgr(CGateService* pGateService);
	~CGateClientSessionMgr();

	CGateClientSession*	createSession(uint64_t nPlayerID, const std::string& szToken);
	CGateClientSession*	getSessionByPlayerID(uint64_t nPlayerID) const;
	CGateClientSession*	getSessionBySocketID(uint64_t nSocketID) const;
	void				destroySession(uint64_t nPlayerID, const std::string& szMsg);

	void				bindSocketID(uint64_t nPlayerID, uint64_t nSocketID);
	void				unbindSocketID(uint64_t nPlayerID);

	uint32_t			getSessionCount() const;

	void				onGasDisconnect(uint32_t nGasID);

private:
	uint32_t								m_nNextSessionID;
	std::map<uint64_t, CGateClientSession*>	m_mapClientSessionByPlayerID;
	std::map<uint64_t, uint64_t>			m_mapPlayerIDBySocketID;
	CGateService*							m_pGateService;
};