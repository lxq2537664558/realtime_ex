#pragma once

#include "libCoreCommon/base_connection_factory.h"

class CLoginClientConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CLoginClientConnectionFactory() { }
	virtual ~CLoginClientConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(const std::string& szType);
};