#pragma once

#include "libCoreCommon/base_connection.h"

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
	bool					registerService(CConnectionFromService* pConnectionFromService, const SServiceBaseInfo& sServiceBaseInfo);
	void					unregisterService(const std::string& szServiceName);
	void					registerMessageInfo(const std::string& szServiceName, const std::vector<SMessageSyncInfo>& vecMessageSyncInfo);

private:
	struct SServiceInfo
	{
		SServiceBaseInfo			sServiceBaseInfo;
		std::set<std::string>		setServiceMessageName;
		CConnectionFromService*		pConnectionFromService;
	};

	std::map<uint32_t, std::string>		m_mapMessageName;
	std::map<std::string, SServiceInfo>	m_mapServiceInfo;
};