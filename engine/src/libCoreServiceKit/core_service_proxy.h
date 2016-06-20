#pragma once

#include "libCoreCommon/core_common.h"
#include "libBaseCommon/noncopyable.h"

#include "tinyxml2/tinyxml2.h"

#include "core_service_kit_define.h"
#include "core_service_connection.h"

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
		
		void						addService(const SServiceBaseInfo& sServiceBaseInfo);
		void						delService(const std::string& szServiceName);

		const SServiceBaseInfo*		getServiceBaseInfo(const std::string& szServiceName) const;
		CCoreServiceConnection*		getServiceConnection(const std::string& szName) const;
		bool						addServiceConnection(CCoreServiceConnection* pCoreConnectionToService);
		void						delServiceConnection(const std::string& szName);

	private:
		std::map<std::string, SServiceBaseInfo>			m_mapServiceBaseInfo;
		std::map<std::string, CCoreServiceConnection*>	m_mapCoreServiceConnection;
	};
}