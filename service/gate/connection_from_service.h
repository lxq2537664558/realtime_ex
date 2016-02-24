#pragma once

#include "libCoreCommon/base_connection.h"

class CConnectionFromService
	: public core::CBaseConnection
{
	DECLARE_OBJECT(CConnectionFromService)

public:
	CConnectionFromService();
	virtual ~CConnectionFromService();

	virtual void		onConnect(const std::string& szContext);
	virtual void		onDisconnect();
	virtual void		onDispatch(uint16_t nMsgType, const void* pData, uint16_t nSize);

	const std::string&	getServiceName() const;

private:
	std::string	m_szServiceName;
};