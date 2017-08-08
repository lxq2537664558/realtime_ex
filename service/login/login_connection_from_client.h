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


	void				setAccountID(uint64_t nAccountID);

private:
	CLoginService*	m_pLoginService;
	uint64_t		m_nAccountID;
};