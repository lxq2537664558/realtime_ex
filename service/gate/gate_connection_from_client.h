#pragma once

#include "libCoreCommon/base_connection.h"

class CGateService;
class CGateConnectionFromClient
	: public core::CBaseConnection
{
public:
	CGateConnectionFromClient();
	virtual ~CGateConnectionFromClient();

	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

private:
	CGateService*	m_pGateService;
};