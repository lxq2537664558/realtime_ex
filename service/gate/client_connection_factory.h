#pragma once

#include "libCoreCommon/base_connection_factory.h"

class CClientConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CClientConnectionFactory() { }
	virtual ~CClientConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(const std::string& szType);
};