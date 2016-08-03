#pragma once

#include "libCoreCommon/base_connection.h"

#include "libCoreServiceKit/core_service_kit_define.h"

#include <vector>
#include <map>
#include <set>

class CConnectionFromService;
class CServiceMgr
{
public:
	CServiceMgr();
	~CServiceMgr();

	bool	init();
	bool	registerService(CConnectionFromService* pConnectionFromService, const core::SServiceBaseInfo& sServiceBaseInfo);
	void	unregisterService(uint16_t nServiceID);
	
private:
	std::map<uint16_t, core::SServiceBaseInfo>	m_mapServiceInfo;
	std::set<std::string>						m_setServiceName;
};