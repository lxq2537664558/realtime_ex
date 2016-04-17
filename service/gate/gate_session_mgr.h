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

	CGateSession*	createSession(uint64_t nSessionID);
	CGateSession*	getSession(uint64_t nSessionID) const;
	void			delSession(uint64_t nSessionID);

private:
	std::map<uint64_t, CGateSession*>	m_mapGateSession;
};