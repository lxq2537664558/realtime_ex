#pragma once
#include "libCoreCommon/core_common.h"

class CGateSession
{
public:
	CGateSession(uint64_t nSessionID);
	~CGateSession();

	uint64_t	getSessionID() const;
	uint64_t	getSocketID() const;

private:
	uint64_t	m_nSessionID;
	uint64_t	m_nSocketID;
};