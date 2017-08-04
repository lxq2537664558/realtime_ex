#include "stdafx.h"
#include "service_registry.h"
#include "connection_from_node.h"
#include "master_service.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libBaseCommon/base_function.h"
#include "libCoreCommon/proto_system.h"
#include "libCoreCommon/base_app.h"

using namespace core;

CServiceRegistry::CServiceRegistry(CMasterService* pMasterService)
	: m_pMasterService(pMasterService)
{

}

CServiceRegistry::~CServiceRegistry()
{

}

bool CServiceRegistry::addNode(CConnectionFromNode* pConnectionFromNode, const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo)
{
	DebugAstEx(pConnectionFromNode != nullptr, false);

	auto iter = this->m_mapNodeInfo.find(sNodeBaseInfo.nID);
	if (iter != this->m_mapNodeInfo.end())
	{
		PrintWarning("CServiceRegistry::addNode dup node id: %d", sNodeBaseInfo.nID);
		return false;
	}

	for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
	{
		const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];
		if (this->m_setServiceName.find(sServiceBaseInfo.szName) != this->m_setServiceName.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service name: %s node id: %d", sServiceBaseInfo.szName.c_str(), sNodeBaseInfo.nID);
			return false;
		}

		if (this->m_setServiceID.find(sServiceBaseInfo.nID) != this->m_setServiceID.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service id: %s node id: %d", sServiceBaseInfo.nID, sNodeBaseInfo.nID);
			return false;
		}
	}

	SNodeInfo sNodeInfo;
	sNodeInfo.sNodeBaseInfo = sNodeBaseInfo;
	sNodeInfo.vecServiceBaseInfo = vecServiceBaseInfo;
	sNodeInfo.pConnectionFromNode = pConnectionFromNode;
	this->m_mapNodeInfo[sNodeBaseInfo.nID] = sNodeInfo;

	for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
	{
		const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];

		this->m_setServiceID.insert(sServiceBaseInfo.nID);
		this->m_setServiceName.insert(sServiceBaseInfo.szName);
	}

	for (auto iter = this->m_mapNodeInfo.begin(); iter != this->m_mapNodeInfo.end(); ++iter)
	{
		const SNodeInfo& sNodeProxyInfo = iter->second;
		if (sNodeProxyInfo.pConnectionFromNode == pConnectionFromNode)
			continue;
		if (sNodeProxyInfo.pConnectionFromNode == nullptr)
			continue;

		base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();

		// 同步基本服务信息
		smt_sync_node_base_info netMsg1;
		netMsg1.sNodeBaseInfo = sNodeProxyInfo.sNodeBaseInfo;
		netMsg1.vecServiceBaseInfo = sNodeProxyInfo.vecServiceBaseInfo;

		netMsg1.pack(writeBuf);
		pConnectionFromNode->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	// 把这个新加入的服务广播给其他服务
	base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();
	
	smt_sync_node_base_info netMsg;
	netMsg.sNodeBaseInfo = sNodeBaseInfo;
	netMsg.vecServiceBaseInfo = vecServiceBaseInfo;

	netMsg.pack(writeBuf);

	std::vector<uint64_t> vecExcludeID;
	vecExcludeID.push_back(pConnectionFromNode->getID());
	CBaseApp::Inst()->getBaseConnectionMgr()->broadcast("CConnectionFromNode", eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), &vecExcludeID);
	
	PrintInfo("register node node_id: %d node_name: %s local addr: %s %d remote addr: %s %d", sNodeBaseInfo.nID, sNodeBaseInfo.szName.c_str(), pConnectionFromNode->getLocalAddr().szHost, pConnectionFromNode->getLocalAddr().nPort, pConnectionFromNode->getRemoteAddr().szHost, pConnectionFromNode->getRemoteAddr().nPort);

	return true;
}

void CServiceRegistry::delNode(uint32_t nNodeID)
{
	auto iter = this->m_mapNodeInfo.find(nNodeID);
	if (iter == this->m_mapNodeInfo.end())
	{
		PrintWarning("CServiceRegistry::delNode unknown node id: %d", nNodeID);
		return;
	}

	const SNodeInfo& sNodeInfo = iter->second;

	base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();

	smt_remove_node_base_info netMsg;
	netMsg.nNodeID = nNodeID;

	netMsg.pack(writeBuf);

	CBaseApp::Inst()->getBaseConnectionMgr()->broadcast("CConnectionFromNode", eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), nullptr);

	for (auto iter = sNodeInfo.vecServiceBaseInfo.begin(); iter != sNodeInfo.vecServiceBaseInfo.end(); ++iter)
	{
		this->m_setServiceID.erase(iter->nID);
		this->m_setServiceName.erase(iter->szName);
	}

	this->m_mapNodeInfo.erase(iter);

	PrintInfo("unregister node node_id: %d", nNodeID);
}
