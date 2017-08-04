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

		virtual CBaseConnection*	createBaseConnection(const std::string& szType);
	};
}