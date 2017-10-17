#pragma once
#include <stdint.h>
#include <string>

#include "core_common.h"

namespace core
{
	enum EServiceSelectorType
	{
		eSST_Random = 1,
		eSST_Hash = 2,
		eSST_RoundRobin = 3,
	};

	class CServiceBase;
	class CServiceSelector
	{
	public:
		CServiceSelector(CServiceBase* pServiceBase) : m_pServiceBase(pServiceBase) { }
		virtual ~CServiceSelector() { }

		CServiceBase*		getServiceBase() const { return this->m_pServiceBase; }

		virtual uint32_t	select(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nContext) = 0;

		// 如果在select中已经考虑到过载保护问题，不希望自己选择出来的服务被过载保护过滤掉，就返回false，比如随机，轮转都自己考虑的过载保护的问题，hash就没有考虑
		virtual bool		isCheckHealth() const { return true; }

	protected:
		CServiceBase*	m_pServiceBase;
	};
}