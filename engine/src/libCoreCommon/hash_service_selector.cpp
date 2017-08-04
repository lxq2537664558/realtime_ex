#include "stdafx.h"
#include "hash_service_selector.h"
#include "core_app.h"

#include "libBaseCommon\rand_gen.h"

namespace core
{
	uint32_t CHashServiceSelector::select(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nContext)
	{
		if (nContext == 0)
			return 0;

		const std::vector<uint32_t>& vecServiceID = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceIDByTypeName(szServiceType);
		if (vecServiceID.empty())
			return 0;

		uint32_t nIndex = (uint32_t)(nContext % vecServiceID.size());
		
		return vecServiceID[nIndex];
	}
}