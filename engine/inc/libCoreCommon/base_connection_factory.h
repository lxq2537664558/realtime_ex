#pragma once

#include "base_connection.h"

namespace core
{
	class CBaseConnectionFactory :
		public base::noncopyable
	{
	public:
		CBaseConnectionFactory() { }
		virtual ~CBaseConnectionFactory() { }

		virtual CBaseConnection*	createBaseConnection(uint32_t nType, const std::string& szContext) = 0;
	};
}