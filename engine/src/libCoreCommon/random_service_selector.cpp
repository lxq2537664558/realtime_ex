#include "stdafx.h"
#include "random_service_selector.h"
#include "service_base.h"
#include "base_app.h"

#include "libBaseCommon/rand_gen.h"

namespace core
{
	CRandomServiceSelector::CRandomServiceSelector(CServiceBase* pServiceBase)
		: CServiceSelector(pServiceBase)
	{

	}

	CRandomServiceSelector::~CRandomServiceSelector()
	{

	}

	uint32_t CRandomServiceSelector::select(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nContext)
	{
		const std::vector<uint32_t>& vecServiceID = this->m_pServiceBase->getActiveServiceIDByType(szServiceType);
		if (vecServiceID.empty())
			return 0;

		for (size_t i = 0; i < 10; ++i)
		{
			uint32_t nIndex = base::CRandGen::getGlobalRand((uint32_t)vecServiceID.size());
			DebugAstEx(nIndex < vecServiceID.size(), 0);

			uint32_t nServiceID = vecServiceID[nIndex];
			if (!this->getServiceBase()->isServiceHealth(nServiceID))
				continue;

			return nServiceID;
		}

		return 0;
	}
}