#include "stdafx.h"
#include "service_registry.h"
#include "connection_from_node.h"
#include "master_service.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libBaseCommon/function_util.h"
#include "libCoreCommon/proto_system.h"
#include "libCoreCommon/base_app.h"

using namespace core;

namespace 
{
	bool isExcludeConnectNode(const std::map<std::string, uint32_t>& mapServiceName, const std::map<std::string, std::set<uint32_t>>& mapServiceType, const CServiceRegistry::SNodeProxyInfo& sSrcNodeProxyInfo, const CServiceRegistry::SNodeProxyInfo& sDstNodeProxyInfo)
	{
		auto callback = [&mapServiceName, &mapServiceType](const CServiceRegistry::SNodeProxyInfo& sSrcNodeProxyInfo, const CServiceRegistry::SNodeProxyInfo& sDstNodeProxyInfo)->bool
		{
			for (auto iter = sSrcNodeProxyInfo.setConnectServiceName.begin(); iter != sSrcNodeProxyInfo.setConnectServiceName.end(); ++iter)
			{
				auto iterNodeID = mapServiceName.find(*iter);
				if (iterNodeID == mapServiceName.end())
					continue;

				if (iterNodeID->second == sDstNodeProxyInfo.sNodeBaseInfo.nID)
					return true;
			}

			for (auto iter = sSrcNodeProxyInfo.setConnectServiceName.begin(); iter != sSrcNodeProxyInfo.setConnectServiceName.end(); ++iter)
			{
				auto iterNodeID = mapServiceType.find(*iter);
				if (iterNodeID == mapServiceType.end())
					continue;

				const auto& setNodeID = iterNodeID->second;
				if (setNodeID.find(sDstNodeProxyInfo.sNodeBaseInfo.nID) != setNodeID.end())
					return true;
			}

			return false;
		};

		if (!callback(sSrcNodeProxyInfo, sDstNodeProxyInfo))
			return false;
		
		if (!callback(sDstNodeProxyInfo, sSrcNodeProxyInfo))
			return false;

		return sSrcNodeProxyInfo.sNodeBaseInfo.nID >= sDstNodeProxyInfo.sNodeBaseInfo.nID;
	}
}

CServiceRegistry::CServiceRegistry(CMasterService* pMasterService)
	: m_pMasterService(pMasterService)
{

}

CServiceRegistry::~CServiceRegistry()
{

}

bool CServiceRegistry::addNode(CConnectionFromNode* pConnectionFromNode, const core::SNodeBaseInfo& sNodeBaseInfo, const std::vector<core::SServiceBaseInfo>& vecServiceBaseInfo, const std::set<std::string>& setConnectServiceName, const std::set<std::string>& setConnectServiceType)
{
	DebugAstEx(pConnectionFromNode != nullptr, false);

	// ���ڵ��Ƿ��ظ�
	auto iter = this->m_mapNodeProxyInfo.find(sNodeBaseInfo.nID);
	if (iter != this->m_mapNodeProxyInfo.end())
	{
		PrintWarning("CServiceRegistry::addNode dup node id: {}", sNodeBaseInfo.nID);
		return false;
	}

	std::map<uint32_t, SServiceBaseInfo> mapServiceBaseInfoByID;
	std::map<std::string, SServiceBaseInfo> mapServiceBaseInfoByName;
	// �������Ƿ��ظ�
	for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
	{
		const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];
		if (this->m_mapServiceName.find(sServiceBaseInfo.szName) != this->m_mapServiceName.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service name: {} node id: {}", sServiceBaseInfo.szName, sNodeBaseInfo.nID);
			return false;
		}

		if (this->m_setServiceID.find(sServiceBaseInfo.nID) != this->m_setServiceID.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service id: {} node id: {}", sServiceBaseInfo.nID, sNodeBaseInfo.nID);
			return false;
		}

		if (mapServiceBaseInfoByID.find(sServiceBaseInfo.nID) != mapServiceBaseInfoByID.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service on same node service_id: {} node id: {}", sServiceBaseInfo.nID, sNodeBaseInfo.nID);
			return false;
		}

		if (mapServiceBaseInfoByName.find(sServiceBaseInfo.szName) != mapServiceBaseInfoByName.end())
		{
			PrintWarning("CServiceRegistry::addNode dup service on same node service_id: {} node id: {}", sServiceBaseInfo.nID, sNodeBaseInfo.nID);
			return false;
		}
	}

	SNodeProxyInfo sNodeProxyInfo;
	sNodeProxyInfo.sNodeBaseInfo = sNodeBaseInfo;
	sNodeProxyInfo.vecServiceBaseInfo = vecServiceBaseInfo;
	sNodeProxyInfo.mapServiceBaseInfo = mapServiceBaseInfoByID;
	sNodeProxyInfo.setConnectServiceName = setConnectServiceName;
	sNodeProxyInfo.setConnectServiceType = setConnectServiceType;
	sNodeProxyInfo.pConnectionFromNode = pConnectionFromNode;
	
	this->m_mapNodeProxyInfo[sNodeBaseInfo.nID] = sNodeProxyInfo;

	for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
	{
		const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];

		this->m_setServiceID.insert(sServiceBaseInfo.nID);
		this->m_mapServiceName[sServiceBaseInfo.szName] = sNodeBaseInfo.nID;
		this->m_mapServiceType[sServiceBaseInfo.szType].insert(sNodeBaseInfo.nID);
	}

	// �Ƚ������ڵ����Ϣ�Լ��������Ϣͬ������ע��Ľڵ㣬������Ҫ��smt_sync_all_node_base_info����ȻһЩ�ڽڵ���ʱ�����ڼ����ߵķ���͵ò���֪ͨ
	smt_sync_all_node_base_info netMsg;
	for (auto iter = this->m_mapNodeProxyInfo.begin(); iter != this->m_mapNodeProxyInfo.end(); ++iter)
	{
		const SNodeProxyInfo& sOtherNodeProxyInfo = iter->second;
		if (sOtherNodeProxyInfo.sNodeBaseInfo.nID == sNodeBaseInfo.nID)
			continue;

		// ���໥��Ҫ���ӶԷ��ģ��ڵ�id��С�ķ�����������
		if (isExcludeConnectNode(this->m_mapServiceName, this->m_mapServiceType, sNodeProxyInfo, sOtherNodeProxyInfo))
			netMsg.setExcludeConnectNodeID.insert(sOtherNodeProxyInfo.sNodeBaseInfo.nID);

		SNodeInfo sNodeSyncInfo;
		sNodeSyncInfo.sNodeBaseInfo = sOtherNodeProxyInfo.sNodeBaseInfo;
		sNodeSyncInfo.vecServiceBaseInfo = sOtherNodeProxyInfo.vecServiceBaseInfo;
		netMsg.mapNodeInfo[sNodeSyncInfo.sNodeBaseInfo.nID] = sNodeSyncInfo;
	}
	
	base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();
	netMsg.pack(writeBuf);
	pConnectionFromNode->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

	// ������¼���Ľڵ��Լ��������Ϣ�㲥�������ڵ㣬���ﲻ�ù㲥�ķ�ʽ����Ϊ�����������Ͼ�����һ�������Ľڵ����ӣ�����Ҫͬ��һЩ��Ϣ����������
	for (auto iter = this->m_mapNodeProxyInfo.begin(); iter != this->m_mapNodeProxyInfo.end(); ++iter)
	{
		const SNodeProxyInfo& sOtherNodeProxyInfo = iter->second;
		if (sOtherNodeProxyInfo.sNodeBaseInfo.nID == sNodeBaseInfo.nID)
			continue;

		if (sOtherNodeProxyInfo.pConnectionFromNode == nullptr)
			continue;

		smt_sync_node_base_info netMsg;
		netMsg.sNodeBaseInfo = sNodeBaseInfo;
		netMsg.vecServiceBaseInfo = vecServiceBaseInfo;
		// ���໥��Ҫ���ӶԷ��ģ��ڵ�id��С�ķ�����������
		if (isExcludeConnectNode(this->m_mapServiceName, this->m_mapServiceType, sOtherNodeProxyInfo, sNodeProxyInfo))
			netMsg.bExcludeConnect = true;
		else
			netMsg.bExcludeConnect = false;

		netMsg.pack(writeBuf);

		sOtherNodeProxyInfo.pConnectionFromNode->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	PrintInfo("register node node_id: {} node_name: {} node_group: {} local addr: {} {} remote addr: {} {}", sNodeBaseInfo.nID, sNodeBaseInfo.szName, sNodeBaseInfo.szGroup, pConnectionFromNode->getLocalAddr().szHost, pConnectionFromNode->getLocalAddr().nPort, pConnectionFromNode->getRemoteAddr().szHost, pConnectionFromNode->getRemoteAddr().nPort);

	return true;
}

void CServiceRegistry::delNode(uint32_t nNodeID)
{
	auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
	if (iter == this->m_mapNodeProxyInfo.end())
	{
		PrintWarning("CServiceRegistry::delNode unknown node id: {}", nNodeID);
		return;
	}

	const SNodeProxyInfo& sNodeProxyInfo = iter->second;

	std::string szNodeName = sNodeProxyInfo.sNodeBaseInfo.szName;
	std::string szNodeGroup = sNodeProxyInfo.sNodeBaseInfo.szGroup;

	for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
	{
		const SServiceBaseInfo& sServiceBaseInfo = sNodeProxyInfo.vecServiceBaseInfo[i];

		this->m_setServiceID.erase(sServiceBaseInfo.nID);
		this->m_mapServiceName.erase(sServiceBaseInfo.szName);
		auto& setServiceID = this->m_mapServiceType[sServiceBaseInfo.szType];
		setServiceID.erase(sServiceBaseInfo.nID);
	}

	this->m_mapNodeProxyInfo.erase(iter);

	smt_remove_node_base_info netMsg;
	netMsg.nNodeID = nNodeID;

	base::CWriteBuf& writeBuf = this->m_pMasterService->getWriteBuf();
	netMsg.pack(writeBuf);

	// ������¼���Ľڵ��Լ��������Ϣ�㲥�������ڵ�
	for (auto iter = this->m_mapNodeProxyInfo.begin(); iter != this->m_mapNodeProxyInfo.end(); ++iter)
	{
		const SNodeProxyInfo& sOtherNodeProxyInfo = iter->second;

		if (sOtherNodeProxyInfo.pConnectionFromNode == nullptr)
			continue;

		sOtherNodeProxyInfo.pConnectionFromNode->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());
	}

	PrintInfo("unregister node node_id: {} node_name: {} node_group: {}", nNodeID, szNodeName, szNodeGroup);
}
