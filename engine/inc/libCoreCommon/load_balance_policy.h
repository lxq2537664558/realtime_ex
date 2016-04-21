#pragma once
#include "core_common.h"

namespace core
{
	class ILoadBalancePolicy
	{
	public:
		virtual ~ILoadBalancePolicy() { }
		
		virtual uint32_t	getID() const = 0;
		virtual std::string	select(const std::string& szMessageName, bool bGate, uint64_t nContext) = 0;
	};

	class CLoadBalanceRandPolicy :
		public ILoadBalancePolicy
	{
	public:
		CLoadBalanceRandPolicy();
		virtual ~CLoadBalanceRandPolicy();

		virtual uint32_t	getID() const;
		virtual std::string select(const std::string& szMessageName, bool bGate, uint64_t nContext);
	};
}