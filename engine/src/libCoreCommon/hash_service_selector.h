#pragma once

#include "service_selector.h"

namespace core
{
	class CHashServiceSelector :
		public CServiceSelector
	{
	public:
		CHashServiceSelector() { }
		virtual ~CHashServiceSelector() { }

		virtual uint32_t	select(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nContext);
	};
}