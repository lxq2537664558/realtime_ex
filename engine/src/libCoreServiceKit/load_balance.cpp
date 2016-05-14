#include "stdafx.h"
#include "load_balance.h"
#include "core_service_kit.h"

#include "libBaseCommon/rand_gen.h"

namespace core
{
	CRandomLoadBalance::CRandomLoadBalance()
	{
	}

	CRandomLoadBalance::~CRandomLoadBalance()
	{

	}

	const std::string& CRandomLoadBalance::getName() const
	{
		return "random";
	}

	std::string CRandomLoadBalance::select(const std::string& szMessageName, uint64_t nSessionID, const std::string& szServiceGroup)
	{
		const SMessageProxyGroupInfo* pMessageProxyGroupInfo = CCoreServiceKit::Inst()->getMessageProxyGroupInfo(szMessageName);
		if (pMessageProxyGroupInfo == nullptr)
			return "";

		uint32_t nTotalWeight = 0;
		if (!szServiceGroup.empty())
		{
			auto iter = pMessageProxyGroupInfo->mapGroupWeight.find(szServiceGroup);
			if (iter == pMessageProxyGroupInfo->mapGroupWeight.end())
				return "";

			nTotalWeight = iter->second;
		}
		else
		{
			nTotalWeight = pMessageProxyGroupInfo->nTotalWeight;
		}

		uint32_t nWeight = base::CRandGen::getGlobalRand(0, nTotalWeight);

		uint32_t nCurWeight = 0;
		for (auto iter = pMessageProxyGroupInfo->mapMessageProxyInfo.begin(); iter != pMessageProxyGroupInfo->mapMessageProxyInfo.end(); ++iter)
		{
			if (!szServiceGroup.empty() && iter->second.szServiceGroup != szServiceGroup)
				continue;

			nCurWeight += iter->second.nWeight;
			if (nCurWeight >= nWeight)
				return iter->second.szServiceName;
		}

		return "";
	}
}