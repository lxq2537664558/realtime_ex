#pragma once

#include "libCoreCommon/base_connection_factory.h"

class CGateClientConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CGateClientConnectionFactory() { }
	virtual ~CGateClientConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(const std::string& szType);
};