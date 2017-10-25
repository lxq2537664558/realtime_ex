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

	bool	addNode(CConnectionFromNode* pConnectionFromNode, const core::SNodeBaseInfo& sNodeBaseInfo, const std::vector<core::SServiceBaseInfo>& vecServiceBaseInfo, const std::set<std::string>& setConnectServiceName, const std::set<std::string>& setConnectServiceType);
	void	delNode(uint32_t nNodeID);
	
public:
	struct SNodeProxyInfo
	{
		core::SNodeBaseInfo		sNodeBaseInfo;
		std::map<uint32_t, core::SServiceBaseInfo>	
								mapServiceBaseInfo;
		std::vector<core::SServiceBaseInfo>
								vecServiceBaseInfo;
		std::set<std::string>	setConnectServiceName;
		std::set<std::string>	setConnectServiceType;
		CConnectionFromNode*	pConnectionFromNode;
	};

private:
	std::map<uint32_t, SNodeProxyInfo>	m_mapNodeProxyInfo;
	std::map<std::string, uint32_t>		m_mapServiceName;	// 服务名字到节点id的映射
	std::map<std::string, std::set<uint32_t>>
										m_mapServiceType;	// 服务类型到节点id的映射
	std::set<uint32_t>					m_setServiceID;

	CMasterService*						m_pMasterService;
};