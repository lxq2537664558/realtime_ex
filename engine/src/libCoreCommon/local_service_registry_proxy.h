#pragma once

#include "libBaseCommon/noncopyable.h"

#include "global_service_registry_proxy.h"


namespace core
{
	// 服务本地的ServiceRegistryProxy，这样设计是为了避免锁竞争
	class CLocalServiceRegistryProxy
	{
		friend class CGlobalServiceRegistryProxy;

	public:
		CLocalServiceRegistryProxy();
		~CLocalServiceRegistryProxy();

		uint32_t		getServiceID(const std::string& szName);
		std::string		getServiceType(uint32_t nServiceID);
		std::string		getServiceName(uint32_t nServiceID);
		const std::set<uint32_t>&
						getServiceIDByType(const std::string& szName);
		const std::vector<uint32_t>&
						getActiveServiceIDByType(const std::string& szName);

		bool			isValidService(uint32_t nServiceID) const;

		const SServiceBaseInfo*
						getServiceBaseInfoByServiceID(uint32_t nServiceID);
		
		uint64_t		getOtherNodeSocketIDByServiceID(uint32_t nServiceID);
		
	private:

#define SServiceProxyInfo CGlobalServiceRegistryProxy::SServiceProxyInfo
#define SServiceIDInfo CGlobalServiceRegistryProxy::SServiceIDInfo

		std::map<uint32_t, SServiceProxyInfo>	m_mapServiceProxyInfo;
		std::map<std::string, SServiceIDInfo>	m_mapServiceIDInfoByType;
		std::map<std::string, uint32_t>			m_mapServiceNameByID;
		std::map<uint32_t, std::string>			m_mapServiceIDByName;
	};
}