#pragma once

#include "libCoreCommon/base_connection.h"

#define eBCT_ConnectionFromClient	_BASE_CONNECTION_TYPE_BEGIN

class CConnectionFromService;
class CConnectionFromClient
	: public core::CBaseConnection
{
public:
	CConnectionFromClient();
	virtual ~CConnectionFromClient();

	virtual bool		init(const std::string& szContext);
	virtual uint32_t	getType() const;
	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual bool		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);
};