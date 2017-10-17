#pragma once

#include "service_selector.h"

namespace core
{
	class CRoundRobinServiceSelector :
		public CServiceSelector
	{
	public:
		CRoundRobinServiceSelector(CServiceBase* pServiceBase);
		virtual ~CRoundRobinServiceSelector();

		virtual uint32_t	select(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nContext);

		virtual bool		isCheckHealth() const { return false; }

	private:
		uint32_t	m_nNextIndex;
	};
}