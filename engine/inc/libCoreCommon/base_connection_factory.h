#pragma once

#include "base_connection.h"

namespace core
{
	class CBaseConnectionFactory
	{
	public:
		CBaseConnectionFactory() { }
		virtual ~CBaseConnectionFactory() { }

		virtual CBaseConnection*	createBaseConnection(const std::string& szType) = 0;
	};
}