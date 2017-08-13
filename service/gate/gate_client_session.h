#pragma once
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/service_invoke_holder.h"

enum EClientSessionState
{
	eCSS_None			= 0,
	eCSS_ClientEnter	= 1<<1,
	eCSS_TokenEnter		= 1<<2,
	eCSS_Normal			= 1<<3,
};

class CGateService;
class CGateClientSession :
	public core::CServiceInvokeHolder
{
public:
	CGateClientSession(CGateService* pGateService);
	~CGateClientSession();

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

	void		enterGas();

private:
	void		onTokenTimeout(uint64_t nContext);

	CGateService*
				getGateService() const;

private:
	uint64_t			m_nSessionID;
	uint64_t			m_nSocketID;
	std::string			m_szToken;
	uint32_t			m_nServiceID;
	uint64_t			m_nPlayerID;
	uint32_t			m_nState;

	core::CTicker		m_tickerTokenTimeout;
};