#pragma once

#include "service_base.h"

namespace core
{
	class CServiceFactory :
		public base::noncopyable
	{
	public:
		CServiceFactory() { }
		virtual ~CServiceFactory() { }

		virtual CServiceBase*	createService(uint16_t nServiceID) = 0;
	};
}