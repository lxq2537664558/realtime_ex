#pragma once

#include "libCoreCommon/base_connection.h"

#include "libCoreServiceKit/core_service_kit_define.h"

#include <vector>
#include <map>

class CConnectionFromService;
class CServiceMgr
{
public:
	CServiceMgr();
	~CServiceMgr();

	bool					init();
	CConnectionFromService*	getServiceConnection(const std::string& szName) const;
	bool					registerService(CConnectionFromService* pConnectionFromService, const core::SServiceBaseInfo& sServiceBaseInfo);
	void					unregisterService(const std::string& szServiceName);
	
private:
	struct SServiceInfo
	{
		core::SServiceBaseInfo							sServiceBaseInfo;
		std::map<std::string, core::SMessageProxyInfo>	mapMessageProxyName;
		CConnectionFromService*							pConnectionFromService;
	};

	std::map<uint32_t, std::string>		m_mapMessageName;
	std::map<std::string, SServiceInfo>	m_mapServiceInfo;
};