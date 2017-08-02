#pragma once

#include "libCoreCommon/base_connection.h"

class CGateService;
class CConnectionFromService;
class CConnectionFromClient
	: public core::CBaseConnection
{
public:
	CConnectionFromClient();
	virtual ~CConnectionFromClient();

	virtual uint32_t	getType() const;
	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

private:
	CGateService*	m_pGateService;
};