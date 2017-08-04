#pragma once

#include "core_service.h"

#include <list>
#include <map>

#include "tinyxml2\tinyxml2.h"

namespace core
{
	class CCoreServiceMgr
	{
	public:
		CCoreServiceMgr();
		~CCoreServiceMgr();
		
		bool				init(tinyxml2::XMLElement* pNodeInfoXML);

		bool				onInit();

		CCoreService*		getCoreServiceByID(uint32_t nID) const;
		CCoreService*		getCoreServiceByName(const std::string& szName) const;
		const std::vector<CCoreService*>&
							getCoreService() const;
		const std::vector<SServiceBaseInfo>&
							getServiceBaseInfo() const;

		bool				isLocalService(uint32_t nServiceID) const;

	private:
		std::vector<SServiceBaseInfo>			m_vecServiceBaseInfo;
		std::map<uint32_t, CCoreService*>		m_mapCoreService;
		std::map<std::string, uint32_t>			m_mapServiceName;
		std::vector<CCoreService*>				m_vecCoreService;
	};
}