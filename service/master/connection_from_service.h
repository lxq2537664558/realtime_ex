#pragma once

#include "libCoreCommon/base_connection.h"

#define eBCT_ConnectionFromService	_BASE_CONNECTION_TYPE_BEGIN

class CConnectionFromService
	: public core::CBaseConnection
{
public:
	CConnectionFromService();
	virtual ~CConnectionFromService();

	virtual bool		init(const std::string& szContext);
	virtual uint32_t	getType() const;
	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual bool		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

	uint16_t			getServiceID() const;

private:
	uint16_t	m_nServiceID;
};