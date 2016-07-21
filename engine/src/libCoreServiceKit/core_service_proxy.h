#pragma once

#include "libCoreCommon/core_common.h"
#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "core_service_kit_define.h"
#include "core_connection_to_service.h"
#include "core_connection_from_service.h"

#include <map>
#include <string>

namespace core
{
	// 其他服务在本地服务的代理
	class CCoreServiceProxy :
		public base::noncopyable
	{
	public:
		CCoreServiceProxy();
		~CCoreServiceProxy();

		bool						init();
		
		void						addServiceBaseInfo(const SServiceBaseInfo& sServiceBaseInfo);
		void						delServiceBaseInfo(const std::string& szServiceName);

		const SServiceBaseInfo*		getServiceBaseInfo(const std::string& szServiceName) const;
		
		CCoreConnectionToService*	getCoreConnectionToService(const std::string& szName) const;
		CCoreConnectionFromService*	getCoreConnectionFromService(const std::string& szName) const;
		
		bool						addCoreConnectionToService(CCoreConnectionToService* pCoreConnectionToService);
		bool						addCoreConnectionFromService(const std::string& szServiceName, CCoreConnectionFromService* pCoreConnectionFromService);
		void						delCoreConnectionToService(const std::string& szServiceName);
		void						delCoreConnectionFromService(const std::string& szServiceName);

	private:
		struct SServiceInfo
		{
			SServiceBaseInfo			sServiceBaseInfo;
			CCoreConnectionToService*	pCoreConnectionToService;
			CCoreConnectionFromService*	pCoreConnectionFromService;
			CTicker*					pTicker;

			void	onTicker(uint64_t nContext);
		};

		std::map<std::string, SServiceInfo>	m_mapServiceInfo;
	};
}