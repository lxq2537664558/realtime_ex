#pragma once

#include "load_balance.h"

#include <map>

namespace core
{
	class CLoadBalanceMgr
	{
	public:
		CLoadBalanceMgr();
		~CLoadBalanceMgr();

		bool			init();
		void			registerLoadBalance(ILoadBalance* pLoadBalance);
		ILoadBalance*	getLoadBalance(const std::string& szName) const;

	private:
		std::map<std::string, ILoadBalance*>	m_mapLoadBalance;
	};
}