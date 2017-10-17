#pragma once

#include "service_selector.h"

namespace core
{
	class CHashServiceSelector :
		public CServiceSelector
	{
	public:
		CHashServiceSelector(CServiceBase* pServiceBase);
		virtual ~CHashServiceSelector();

		virtual uint32_t	select(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nContext);
	};
}