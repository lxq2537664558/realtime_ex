#pragma once

#include "libCoreCommon/base_connection_factory.h"

class CNodeConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CNodeConnectionFactory() { }
	virtual ~CNodeConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(uint32_t nType);
};