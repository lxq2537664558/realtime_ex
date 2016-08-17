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
		void						delServiceBaseInfo(uint16_t nServiceID);

		uint16_t					getServiceID(const std::string& szServiceName) const;

		const SServiceBaseInfo*		getServiceBaseInfo(uint16_t nServiceID) const;
		
		CCoreConnectionToService*	getCoreConnectionToService(uint16_t nServiceID) const;
		CCoreConnectionFromService*	getCoreConnectionFromService(uint16_t nServiceID) const;
		
		bool						addCoreConnectionToService(CCoreConnectionToService* pCoreConnectionToService);
		bool						addCoreConnectionFromService(uint16_t nServiceID, CCoreConnectionFromService* pCoreConnectionFromService);
		void						delCoreConnectionToService(uint16_t nServiceID);
		void						delCoreConnectionFromService(uint16_t nServiceID);

	private:
		struct SServiceInfo
		{
			SServiceBaseInfo			sServiceBaseInfo;
			CCoreConnectionToService*	pCoreConnectionToService;
			CCoreConnectionFromService*	pCoreConnectionFromService;
			std::unique_ptr<CTicker>	pTicker;

			void	onTicker(uint64_t nContext);
		};

		std::map<uint16_t, SServiceInfo>	m_mapServiceInfo;
		std::map<std::string, uint16_t>		m_mapServiceName;
	};
}