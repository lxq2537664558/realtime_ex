#pragma once
#include "libCoreCommon/core_common.h"

enum EClientSessionState
{
	eCSS_None			= 0,
	eCSS_ClientEnter	= 1<<1,
	eCSS_TokenEnter		= 1<<2,
	eCSS_Normal			= 1<<3,
};

class CGateService;
class CClientSession
{
public:
	CClientSession();
	~CClientSession();

	bool		init(uint64_t nPlayerID, uint64_t nSessionID, const std::string& szToken);
	
	uint64_t	getSessionID() const;
	uint64_t	getPlayerID() const;

	uint64_t	getSocketID() const;
	void		setSocketID(uint64_t nSocketID);

	const std::string&
				getToken() const;
	void		setToken(const std::string& szToken);

	uint32_t	getServiceID() const;
	void		setServiceID(uint32_t nServiceID);

	uint32_t	getState() const;
	void		setState(EClientSessionState eState);

	void		enterGas(CGateService* pGateService);

private:
	uint64_t			m_nSessionID;
	uint64_t			m_nSocketID;
	std::string			m_szToken;
	uint32_t			m_nServiceID;
	uint64_t			m_nPlayerID;
	uint32_t			m_nState;
};