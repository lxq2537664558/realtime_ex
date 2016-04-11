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
	void					addService(CConnectionFromService* pConnectionFromService, const SServiceBaseInfo& sServiceBaseInfo);
	void					delService(const std::string& szName);
	void					addServiceMessageInfo(const std::string& szName, const std::vector<SMessageSyncInfo>& vecMessageSyncInfo, bool bAdd);

private:
	struct SServiceInfo
	{
		SServiceBaseInfo		sServiceBaseInfo;
		std::vector<uint32_t>	vecServiceMessageID;
		std::vector<uint32_t>	vecGateServiceMessageID;
		CConnectionFromService*	pConnectionFromService;
	};

	std::map<std::string, SServiceInfo>	m_mapServiceInfo;
};