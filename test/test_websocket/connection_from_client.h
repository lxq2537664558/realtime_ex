#pragma once

#include "libCoreCommon/base_connection.h"

class CConnectionFromClient
	: public core::CBaseConnection
{
public:
	CConnectionFromClient();
	virtual ~CConnectionFromClient();

	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);
};