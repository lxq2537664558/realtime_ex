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

		// �����select���Ѿ����ǵ����ر������⣬��ϣ���Լ�ѡ������ķ��񱻹��ر������˵����ͷ���false�������������ת���Լ����ǵĹ��ر��������⣬hash��û�п���
		virtual bool		isCheckHealth() const { return true; }

	protected:
		CServiceBase*	m_pServiceBase;
	};
}