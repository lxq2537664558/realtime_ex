#pragma once

#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/core_common.h"

#include <vector>
#include <map>
#include <set>

class CConnectionFromNode;
class CMasterService;
class CServiceRegistry
{
public:
	CServiceRegistry(CMasterService* pMasterService);
	~CServiceRegistry();

	bool	addNode(CConnectionFromNode* pConnectionFromNode, const core::SNodeBaseInfo& sNodeBaseInfo, const std::vector<core::SServiceBaseInfo>& vecServiceBaseInfo);
	void	delNode(uint32_t nNodeID);
	
private:
	struct SNodeInfo
	{
		core::SNodeBaseInfo					sNodeBaseInfo;
		std::vector<core::SServiceBaseInfo>	vecServiceBaseInfo;
		CConnectionFromNode*				pConnectionFromNode;
	};

	std::map<uint32_t, SNodeInfo>	m_mapNodeInfo;
	std::set<std::string>			m_setServiceName;
	std::set<uint32_t>				m_setServiceID;

	CMasterService*					m_pMasterService;
};