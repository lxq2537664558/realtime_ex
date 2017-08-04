#pragma once

#include "libCoreCommon/base_connection.h"

class CMasterService;
class CConnectionFromNode
	: public core::CBaseConnection
{
public:
	CConnectionFromNode();
	virtual ~CConnectionFromNode();

	virtual void		release();

	virtual void		onConnect();
	virtual void		onDisconnect();
	virtual void		onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

	uint32_t			getNodeID() const;

private:
	uint32_t		m_nNodeID;
	CMasterService*	m_pMasterService;
};