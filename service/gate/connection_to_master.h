#pragma once

#include "libCoreCommon/base_connection.h"

class CConnectionToMaster
	: public core::CBaseConnection
{
	DECLARE_OBJECT(CConnectionToMaster)

public:
	CConnectionToMaster();
	virtual ~CConnectionToMaster();

	virtual void	onConnect(const std::string& szContext);
	virtual void	onDisconnect();
	virtual void	onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize);
};