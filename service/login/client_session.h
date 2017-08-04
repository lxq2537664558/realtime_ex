#pragma once
#include "libCoreCommon/core_common.h"

enum EClientSessionState
{
	eCSS_None,
	eCSS_WaitEnter,
	eCSS_Normal,
};

class CClientSession
{
public:
	CClientSession();
	~CClientSession();

	bool		init(uint64_t nSocketID, uint64_t nSessionID, uint32_t nServiceID, uint64_t nPlayerID, const std::string& szToken);
	
	uint64_t	getSessionID() const;
	uint64_t	getSocketID() const;

	const std::string&
				getToken() const;
	uint64_t	getPlayerID() const;
	uint32_t	getServiceID() const;

	EClientSessionState
				getState() const;
	void		setState(EClientSessionState eState);

private:
	uint64_t			m_nSessionID;
	uint64_t			m_nSocketID;
	std::string			m_szToken;
	uint32_t			m_nServiceID;
	uint64_t			m_nPlayerID;
	EClientSessionState	m_eState;
};