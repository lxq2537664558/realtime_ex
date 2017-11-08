#pragma once
#include <stdint.h>

#include <string>

#include "libBaseCommon/ticker.h"

class CUCService;
class CUserInfo
{
public:
	CUserInfo(CUCService* pUCService, uint64_t nPlayerID, uint32_t nGasID);
	~CUserInfo();

	uint64_t	getPlayerID() const;
	uint32_t	getGasID() const;

	void		setGasID(uint32_t nGasID);

	void		onHeartbeat();

	void		startTimout();

private:
	void		onTimeout(uint64_t nContext);

private:
	CUCService*		m_pUCService;
	uint64_t		m_nPlayerID;
	uint32_t		m_nGasID;
	base::CTicker	m_tickerTimeout;
};