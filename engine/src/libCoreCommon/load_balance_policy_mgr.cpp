#include "stdafx.h"
#include "load_balance_policy_mgr.h"

namespace core
{
	CLoadBalancePolicyMgr::CLoadBalancePolicyMgr()
	{

	}

	CLoadBalancePolicyMgr::~CLoadBalancePolicyMgr()
	{

	}

	bool CLoadBalancePolicyMgr::init()
	{
		return true;
	}

	void CLoadBalancePolicyMgr::registLoadBalancePolicy(ILoadBalancePolicy* pLoadBalancePolicy)
	{
		DebugAst(pLoadBalancePolicy != nullptr);

		this->m_mapLoadBalancePolicy[pLoadBalancePolicy->getID()] = pLoadBalancePolicy;
	}

	ILoadBalancePolicy* CLoadBalancePolicyMgr::getLoadBalancePolicy(uint32_t nID) const
	{
		auto iter = this->m_mapLoadBalancePolicy.find(nID);
		if (iter == this->m_mapLoadBalancePolicy.end())
			return nullptr;

		return iter->second;
	}
}