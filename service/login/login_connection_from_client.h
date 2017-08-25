#pragma once

#include "libCoreCommon/base_connection.h"

class CLoginService;
class CLoginConnectionFromClient
	: public core::CBaseConnection
{
public:
	CLoginConnectionFromClient();
	virtual ~CLoginConnectionFromClient();

	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);


	void				setAccountInfo(const std::string& szAccountName, uint32_t nServerID);

private:
	CLoginService*	m_pLoginService;
	std::string		m_szAccountName;
	uint32_t		m_nServerID;
};