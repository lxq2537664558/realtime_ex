#pragma once

#include "libCoreCommon/base_connection_factory.h"

class CNodeConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CNodeConnectionFactory() { }
	virtual ~CNodeConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(const std::string& szType);
};