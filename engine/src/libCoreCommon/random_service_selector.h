#pragma once

#include "service_selector.h"

namespace core
{
	class CRandomServiceSelector :
		public CServiceSelector
	{
	public:
		CRandomServiceSelector(CServiceBase* pServiceBase);
		virtual ~CRandomServiceSelector();

		virtual uint32_t	select(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nContext);

		virtual bool		isCheckHealth() const { return false; }
	};
}