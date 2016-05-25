#pragma once

#include "libCoreCommon/base_connection_factory.h"

namespace core
{
	class CServiceConnectionFactory :
		public CBaseConnectionFactory
	{
	public:
		CServiceConnectionFactory() { }
		virtual ~CServiceConnectionFactory() { }

		virtual CBaseConnection*	createBaseConnection(uint32_t nType, const std::string& szContext);
	};
}