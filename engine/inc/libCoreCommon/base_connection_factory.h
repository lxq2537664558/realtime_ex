#pragma once

#include "base_connection.h"

namespace core
{
	class CBaseConnectionFactory
	{
	public:
		CBaseConnectionFactory() { }
		virtual ~CBaseConnectionFactory() { }

		virtual CBaseConnection*	createBaseConnection(uint32_t nType) = 0;
	};
}