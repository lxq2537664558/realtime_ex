#pragma once

#include "libCoreCommon/base_connection.h"

#include "libCoreServiceKit/core_service_kit_define.h"

#include <vector>
#include <map>
#include <set>

class CConnectionFromNode;
class CServiceMgr
{
public:
	CServiceMgr();
	~CServiceMgr();

	bool	init();
	bool	registerNode(CConnectionFromNode* pConnectionFromService, const core::SNodeBaseInfo& sNodeBaseInfo, const std::vector<core::SServiceBaseInfo>& vecServiceBaseInfo);
	void	unregisterNode(uint16_t nNodeID);

private:
	struct SNodeInfo
	{
		core::SNodeBaseInfo					sNodeBaseInfo;
		std::vector<core::SServiceBaseInfo>	vecServiceBaseInfo;
	};
	std::map<uint16_t, SNodeInfo>	m_mapNodeInfo;
	std::set<std::string>			m_setServiceName;
	std::set<uint16_t>				m_setServiceID;
};