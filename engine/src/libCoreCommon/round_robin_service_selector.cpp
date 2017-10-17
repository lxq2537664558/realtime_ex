#include "stdafx.h"
#include "round_robin_service_selector.h"
#include "service_base.h"
#include "base_app.h"

#include "libBaseCommon/rand_gen.h"

namespace core
{
	CRoundRobinServiceSelector::CRoundRobinServiceSelector(CServiceBase* pServiceBase)
		: CServiceSelector(pServiceBase)
	{

	}

	CRoundRobinServiceSelector::~CRoundRobinServiceSelector()
	{

	}

	uint32_t CRoundRobinServiceSelector::select(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nContext)
	{
		const std::vector<uint32_t>& vecServiceID = core::CBaseApp::Inst()->getActiveServiceIDByType(szServiceType);
		if (vecServiceID.empty())
			return 0;

		for (size_t i = 0; i < 10; ++i)
		{
			uint32_t nIndex = (uint32_t)(this->m_nNextIndex % vecServiceID.size());
			++this->m_nNextIndex;

			uint32_t nServiceID = vecServiceID[nIndex];
			if (!this->getServiceBase()->isServiceHealth(nServiceID))
				continue;

			return nServiceID;
		}
		
		return 0;
	}
}