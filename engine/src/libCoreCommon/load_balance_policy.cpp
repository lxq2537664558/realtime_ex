#include "stdafx.h"
#include "load_balance_policy.h"
#include "base_app.h"

#include "libBaseCommon/rand_gen.h"

namespace core
{
	CLoadBalanceRandPolicy::CLoadBalanceRandPolicy()
	{
	}

	CLoadBalanceRandPolicy::~CLoadBalanceRandPolicy()
	{

	}

	uint32_t CLoadBalanceRandPolicy::getID() const
	{
		return eLBPID_Rand;
	}

	std::string CLoadBalanceRandPolicy::select(const std::string& szMessageName, uint64_t nContext)
	{
		const std::set<std::string>& setServiceName = CBaseApp::Inst()->getServiceName(szMessageName);
		uint32_t nIndex = base::CRandGen::getGlobalRand(0, (uint32_t)setServiceName.size());
		
		uint32_t i = 0;
		for (auto iter = setServiceName.begin(); iter != setServiceName.end(); ++iter, ++i)
		{
			if (i == nIndex)
				return *iter;
		}

		return "";
	}
}