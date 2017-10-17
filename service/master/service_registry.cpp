#include "stdafx.h"
#include "service_registry.h"
#include "connection_from_node.h"
#include "master_service.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libBaseCommon/function_util.h"
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
		PrintWarning("CServiceRegistry::addNode dup node id: {}", sNodeBaseInfo.nID);
		return false;
	}

	for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
	{
		const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];
		if (this->m_setServiceName.find(sServiceBaseInfo.szName) != this->m_setServiceName.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service name: {} node id: {}", sServiceBaseInfo.szName, sNodeBaseInfo.nID);
			return false;
		}

		if (this->m_setServiceID.find(sServiceBaseInfo.nID) != this->m_setServiceID.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service id: {} node id: {}", sServiceBaseInfo.nID, sNodeBaseInfo.nID);
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
		const SNodeInfo& sOtherNodeProxyInfo = iter->second;
		if (sOtherNodeProxyInfo.sNodeBaseInfo.nID == sNodeBaseInfo.nID)
			continue;

		if (sOtherNodeProxyInfo.pConnectionFromNode == nullptr)
			continue;

		base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();

		// ͬ������������Ϣ
		smt_sync_node_base_info netMsg1;
		netMsg1.sNodeBaseInfo = sOtherNodeProxyInfo.sNodeBaseInfo;
		netMsg1.vecServiceBaseInfo = sOtherNodeProxyInfo.vecServiceBaseInfo;

		netMsg1.pack(writeBuf);
		pConnectionFromNode->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	// ������¼���ķ���㲥����������
	base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();
	
	smt_sync_node_base_info netMsg;
	netMsg.sNodeBaseInfo = sNodeBaseInfo;
	netMsg.vecServiceBaseInfo = vecServiceBaseInfo;

	netMsg.pack(writeBuf);


	for (auto iter = this->m_mapNodeInfo.begin(); iter != this->m_mapNodeInfo.end(); ++iter)
	{
		const SNodeInfo& sOtherNodeProxyInfo = iter->second;
		if (sOtherNodeProxyInfo.sNodeBaseInfo.nID == sNodeBaseInfo.nID)
			continue;

		if (sOtherNodeProxyInfo.pConnectionFromNode == nullptr)
			continue;

		sOtherNodeProxyInfo.pConnectionFromNode->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	PrintInfo("register node node_id: {} node_name: {} local addr: {} {} remote addr: {} {}", sNodeBaseInfo.nID, sNodeBaseInfo.szName, pConnectionFromNode->getLocalAddr().szHost, pConnectionFromNode->getLocalAddr().nPort, pConnectionFromNode->getRemoteAddr().szHost, pConnectionFromNode->getRemoteAddr().nPort);

	return true;
}

void CServiceRegistry::delNode(uint32_t nNodeID)
{
	auto iter = this->m_mapNodeInfo.find(nNodeID);
	if (iter == this->m_mapNodeInfo.end())
	{
		PrintWarning("CServiceRegistry::delNode unknown node id: {}", nNodeID);
		return;
	}

	const SNodeInfo& sNodeInfo = iter->second;

	base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();

	smt_remove_node_base_info netMsg;
	netMsg.nNodeID = nNodeID;

	netMsg.pack(writeBuf);

	this->m_pMasterService->getBaseConnectionMgr()->broadcast("CConnectionFromNode", eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), nullptr);

	for (auto iter = sNodeInfo.vecServiceBaseInfo.begin(); iter != sNodeInfo.vecServiceBaseInfo.end(); ++iter)
	{
		this->m_setServiceID.erase(iter->nID);
		this->m_setServiceName.erase(iter->szName);
	}

	this->m_mapNodeInfo.erase(iter);

	PrintInfo("unregister node node_id: {}", nNodeID);
}
