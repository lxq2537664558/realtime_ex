#pragma once

#include "core_service.h"

#include <list>
#include <map>

#include "tinyxml2/tinyxml2.h"

namespace core
{
	class CCoreServiceMgr
	{
	public:
		CCoreServiceMgr();
		~CCoreServiceMgr();
		
		bool			init(const std::vector<CServiceBase*>& vecServiceBase);

		bool			onInit();

		CCoreService*	getCoreService(uint32_t nID) const;
		const std::vector<CCoreService*>&
						getCoreService() const;
		std::vector<SServiceBaseInfo>
						getServiceBaseInfo() const;

		bool			isLocalService(uint32_t nServiceID) const;

	private:
		std::map<uint32_t, CCoreService*>		m_mapCoreService;
		std::vector<CCoreService*>				m_vecCoreService;
	};
}