#include "stdafx.h"
#include "hash_service_selector.h"
#include "service_base.h"
#include "base_app.h"

namespace core
{

	CHashServiceSelector::CHashServiceSelector(CServiceBase* pServiceBase)
		: CServiceSelector(pServiceBase)
	{

	}

	CHashServiceSelector::~CHashServiceSelector()
	{

	}

	uint32_t CHashServiceSelector::select(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nContext)
	{
		const std::vector<uint32_t>& vecServiceID = core::CBaseApp::Inst()->getActiveServiceIDByType(szServiceType);
		if (vecServiceID.empty())
			return 0;

		uint32_t nIndex = (uint32_t)(nContext % vecServiceID.size());
		
		return vecServiceID[nIndex];
	}
}