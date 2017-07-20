#pragma once

#include "libCoreCommon/base_connection.h"

#define eBCT_ConnectionFromService	_BASE_CONNECTION_TYPE_BEGIN

class CConnectionFromNode
	: public core::CBaseConnection
{
public:
	CConnectionFromNode();
	virtual ~CConnectionFromNode();

	virtual bool		init(const std::string& szContext);
	virtual uint32_t	getType() const;
	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

	uint16_t			getNodeID() const;

private:
	uint16_t	m_nNodeID;
};