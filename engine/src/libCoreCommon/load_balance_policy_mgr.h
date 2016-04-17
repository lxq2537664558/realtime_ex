#pragma once

#include "load_balance_policy.h"

#include <map>

namespace core
{
	class CLoadBalancePolicyMgr
	{
	public:
		CLoadBalancePolicyMgr();
		~CLoadBalancePolicyMgr();

		bool				init();
		void				registLoadBalancePolicy(ILoadBalancePolicy* pLoadBalancePolicy);
		ILoadBalancePolicy*	getLoadBalancePolicy(uint32_t nID) const;

	private:
		std::map<uint32_t, ILoadBalancePolicy*>	m_mapLoadBalancePolicy;
	};
}