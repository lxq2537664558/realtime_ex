#pragma once

#include "libCoreCommon/base_connection_factory.h"

class CServiceConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CServiceConnectionFactory() { }
	virtual ~CServiceConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(uint32_t nType, const std::string& szContext);
};