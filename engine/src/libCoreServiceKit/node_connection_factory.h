#pragma once

#include "libCoreCommon/base_connection_factory.h"

namespace core
{
	class CNodeConnectionFactory :
		public CBaseConnectionFactory
	{
	public:
		CNodeConnectionFactory() { }
		virtual ~CNodeConnectionFactory() { }

		virtual CBaseConnection*	createBaseConnection(uint32_t nType, const std::string& szContext);
	};
}