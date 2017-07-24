#pragma once

#include "base_connection.h"

#include "libBaseCommon\noncopyable.h"

namespace core
{
	class CBaseConnectionFactory :
		public base::noncopyable
	{
	public:
		CBaseConnectionFactory() { }
		virtual ~CBaseConnectionFactory() { }

		virtual CBaseConnection*	createBaseConnection(uint32_t nType) = 0;
	};
}