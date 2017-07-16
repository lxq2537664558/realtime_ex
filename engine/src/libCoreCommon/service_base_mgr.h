#pragma once

#include "service_base_impl.h"

#include <list>
#include <map>

namespace core
{
	class CServiceBaseMgr
	{
	public:
		CServiceBaseMgr();
		~CServiceBaseMgr();
		
		bool				init(const std::vector<SServiceBaseInfo>& vecServiceBaseInfo);

		CServiceBaseImpl*	getServiceBase(uint16_t nID) const;
		const std::vector<CServiceBaseImpl*>&
							getServiceBase() const;

	private:
		std::map<uint16_t, CServiceBaseImpl*>	m_mapServiceBase;
		std::vector<CServiceBaseImpl*>			m_vecServiceBase;
	};
}