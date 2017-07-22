#pragma once

#include "service_base_impl.h"

#include <list>
#include <map>

#include "tinyxml2\tinyxml2.h"

namespace core
{
	class CServiceBaseMgr
	{
	public:
		CServiceBaseMgr();
		~CServiceBaseMgr();
		
		bool				init(tinyxml2::XMLElement* pNodeInfoXML);

		CServiceBaseImpl*	getServiceBaseByID(uint32_t nID) const;
		CServiceBaseImpl*	getServiceBaseByName(const std::string& szName) const;
		const std::vector<CServiceBaseImpl*>&
							getServiceBase() const;
		const std::vector<SServiceBaseInfo>&
							getServiceBaseInfo() const;

		bool				isOwnerService(uint32_t nServiceID) const;

	private:
		std::vector<SServiceBaseInfo>			m_vecServiceBaseInfo;
		std::map<uint32_t, CServiceBaseImpl*>	m_mapServiceBase;
		std::map<std::string, uint32_t>			m_mapServiceName;
		std::vector<CServiceBaseImpl*>			m_vecServiceBase;
	};
}