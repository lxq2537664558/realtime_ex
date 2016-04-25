#pragma once
#include "libCoreCommon/core_common.h"

class CGateSession
{
public:
	CGateSession();
	~CGateSession();

	bool		init(uint64_t nSocketID, uint64_t nSessionID);
	uint64_t	getSessionID() const;
	uint64_t	getSocketID() const;

private:
	uint64_t	m_nSessionID;
	uint64_t	m_nSocketID;
};