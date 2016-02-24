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
	void					addServiceConnection(CConnectionFromService* pServiceConnection);
	void					delServiceConnection(const std::string& szName);

private:
	std::map<std::string, CConnectionFromService*>	m_mapServiceConnection;
};