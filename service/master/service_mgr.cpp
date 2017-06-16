#include "stdafx.h"
#include "service_mgr.h"
#include "master_app.h"
#include "connection_from_node.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libBaseCommon/base_function.h"
#include "libCoreServiceKit/proto_system.h"
#include "libCoreServiceKit/core_service_kit_define.h"

CServiceMgr::CServiceMgr()
{

}

CServiceMgr::~CServiceMgr()
{

}

bool CServiceMgr::init()
{
	return true;
}

bool CServiceMgr::registerNode(CConnectionFromNode* pConnectionFromNode, const core::SNodeBaseInfo& sNodeBaseInfo, const std::vector<core::SServiceBaseInfo>& vecServiceBaseInfo)
{
	auto iter = this->m_mapNodeInfo.find(sNodeBaseInfo.nID);
	if (iter != this->m_mapNodeInfo.end())
	{
		PrintWarning("dup node id node_id: %d node_name: %s", sNodeBaseInfo.nID, sNodeBaseInfo.szName.c_str());
		return false;
	}

	for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
	{
		if (this->m_setServiceName.find(vecServiceBaseInfo[i].szName) != this->m_setServiceName.end())
		{
			PrintWarning("dup service name node_id: %d node_name: %s service_id: %d service_name: %s", sNodeBaseInfo.nID, sNodeBaseInfo.szName.c_str(), vecServiceBaseInfo[i].nID, vecServiceBaseInfo[i].szName.c_str());
			return false;
		}

		if (this->m_setServiceID.find(vecServiceBaseInfo[i].nID) != this->m_setServiceID.end())
		{
			PrintWarning("dup service id node_id: %d node_name: %s service_id: %d service_name: %s", sNodeBaseInfo.nID, sNodeBaseInfo.szName.c_str(), vecServiceBaseInfo[i].nID, vecServiceBaseInfo[i].szName.c_str());
			return false;
		}
	}

	this->m_mapNodeInfo[sNodeBaseInfo.nID] = { sNodeBaseInfo, vecServiceBaseInfo };
	for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
	{
		this->m_setServiceName.insert(vecServiceBaseInfo[i].szName);
		this->m_setServiceID.insert(vecServiceBaseInfo[i].nID);
	}
	
	std::vector<uint64_t> vecSocketID;
	// 将其他服务的信息同步给新的服务
	std::vector<core::CBaseConnection*> vecBaseConnection = CMasterApp::Inst()->getBaseConnectionMgr()->getBaseConnection(eBCT_ConnectionFromNode);
	for (size_t i = 0; i < vecBaseConnection.size(); ++i)
	{
		core::CBaseConnection* pBaseConnection = vecBaseConnection[i];
		if (nullptr == pBaseConnection)
			continue;

		CConnectionFromNode* pOtherConnectionFromNode = dynamic_cast<CConnectionFromNode*>(pBaseConnection);
		if (nullptr == pOtherConnectionFromNode || pOtherConnectionFromNode == pConnectionFromNode)
			continue;

		auto iter = this->m_mapNodeInfo.find(pOtherConnectionFromNode->getNodeID());
		if (iter == this->m_mapNodeInfo.end())
			continue;

		SNodeInfo& sOtherNodeInfo = iter->second;
		base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

		// 同步基本服务信息
		core::smt_sync_node_base_info netMsg1;
		netMsg1.sNodeBaseInfo = sOtherNodeInfo.sNodeBaseInfo;
		netMsg1.vecServiceBaseInfo = sOtherNodeInfo.vecServiceBaseInfo;

		netMsg1.pack(writeBuf);
		pConnectionFromNode->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		vecSocketID.push_back(pOtherConnectionFromNode->getID());
	}

	// 把这个新加入的服务广播给其他服务
	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

	core::smt_sync_node_base_info netMsg;
	netMsg.vecServiceBaseInfo = vecServiceBaseInfo;
	netMsg.sNodeBaseInfo = sNodeBaseInfo;
	// 监听方跟主动连接方的接收缓存，发送缓存大小对调
	std::swap(netMsg.sNodeBaseInfo.nRecvBufSize, netMsg.sNodeBaseInfo.nSendBufSize);

	netMsg.pack(writeBuf);

	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(vecSocketID, eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

	PrintInfo("register node node_id: %d node_name: %s local addr: %s %d remote addr: %s %d", sNodeBaseInfo.nID, sNodeBaseInfo.szName.c_str(), pConnectionFromNode->getLocalAddr().szHost, pConnectionFromNode->getLocalAddr().nPort, pConnectionFromNode->getRemoteAddr().szHost, pConnectionFromNode->getRemoteAddr().nPort);

	return true;
}

void CServiceMgr::unregisterNode(uint16_t nNodeID)
{
	auto iter = this->m_mapNodeInfo.find(nNodeID);
	if (iter == this->m_mapNodeInfo.end())
		return;

	// 清理该服务的消息
	SNodeInfo& sNodeInfo = iter->second;

	base::CWriteBuf& writeBuf = CMasterApp::Inst()->getWriteBuf();

	core::smt_remove_node_base_info netMsg;
	netMsg.nNodeID = nNodeID;

	netMsg.pack(writeBuf);

	CMasterApp::Inst()->getBaseConnectionMgr()->broadcast(eBCT_ConnectionFromNode, eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize(), nullptr);

	for (size_t i = 0; i < sNodeInfo.vecServiceBaseInfo.size(); ++i)
	{
		this->m_setServiceName.erase(sNodeInfo.vecServiceBaseInfo[i].szName);
		this->m_setServiceID.erase(sNodeInfo.vecServiceBaseInfo[i].nID);
	}
	this->m_setServiceName.erase(sNodeInfo.sNodeBaseInfo.szName);

	PrintInfo("unregister node node_id: %d", nNodeID);
}