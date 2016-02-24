#pragma once

#include "libCoreCommon/base_connection.h"

class CConnectionFromService;
class CConnectionFromClient
	: public core::CBaseConnection
{
	DECLARE_OBJECT(CConnectionFromClient)

public:
	CConnectionFromClient();
	virtual ~CConnectionFromClient();

	virtual void	onConnect(const std::string& szContext);
	virtual void	onDisconnect();
	virtual void	onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize);

private:
	CConnectionFromService*	m_pConnectionFromService;
};