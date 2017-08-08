#include "stdafx.h"
#include "random_service_selector.h"
#include "core_app.h"

#include "libBaseCommon/rand_gen.h"

namespace core
{
	uint32_t CRandomServiceSelector::select(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nContext)
	{
		const std::vector<uint32_t>& vecServiceID = CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceIDByTypeName(szServiceType);
		if (vecServiceID.empty())
			return 0;

		uint32_t nIndex = base::CRandGen::getGlobalRand((uint32_t)vecServiceID.size());
		DebugAstEx(nIndex < vecServiceID.size(), 0);

		return vecServiceID[nIndex];
	}
}