#pragma once

#include "gate_session.h"

#include <map>
using namespace std;

class CGateSessionMgr
{
public:
	CGateSessionMgr();
	~CGateSessionMgr();

	bool			init();

	CGateSession*	createSession(uint64_t nSocketID, uint64_t nSessionID);
	CGateSession*	getSessionBySessionID(uint64_t nSessionID) const;
	CGateSession*	getSessionBySocketID(uint64_t nSocketID) const;
	void			delSessionbySocketID(uint64_t nSocketID);

private:
	std::map<uint64_t, CGateSession*>	m_mapGateSessionBySessionID;
	std::map<uint64_t, uint64_t>		m_mapSessionIDBySocketID;
};