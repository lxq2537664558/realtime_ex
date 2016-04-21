#include "stdafx.h"
#include "load_balance_policy.h"
#include "base_app.h"

#include "libBaseCommon\rand_gen.h"

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

	std::string CLoadBalanceRandPolicy::select(const std::string& szMessageName, bool bGate, uint64_t nContext)
	{
		const std::vector<std::string>& vecServiceName = CBaseApp::Inst()->getServiceName(szMessageName, bGate);
		uint32_t nIndex = base::CRandGen::getGlobalRand(0, (uint32_t)vecServiceName.size());
		DebugAstEx(nIndex < vecServiceName.size(), "");

		return vecServiceName[nIndex];
	}
}