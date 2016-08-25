#include "stdafx.h"
#include "core_other_node_proxy.h"
#include "core_service_app_impl.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#define _CHECK_CONNECT_TIME 5000

namespace core
{

	CCoreOtherNodeProxy::CCoreOtherNodeProxy()
	{

	}

	CCoreOtherNodeProxy::~CCoreOtherNodeProxy()
	{

	}

	bool CCoreOtherNodeProxy::init()
	{
		return true;
	}

	void CCoreOtherNodeProxy::addNodeBaseInfo(const SNodeBaseInfo& sNodeBaseInfo)
	{
		DebugAst(this->m_mapNodeInfo.find(sNodeBaseInfo.nID) == this->m_mapNodeInfo.end());
		DebugAst(this->m_mapNodeName.find(sNodeBaseInfo.szName) == this->m_mapNodeName.end());

		SNodeInfo& sNodeInfo = this->m_mapNodeInfo[sNodeBaseInfo.nID];
		sNodeInfo.pCoreConnectionFromOtherNode = nullptr;
		sNodeInfo.pCoreConnectionToOtherNode = nullptr;
		sNodeInfo.sNodeBaseInfo = sNodeBaseInfo;
		sNodeInfo.pTicker = std::make_unique<CTicker>();
		sNodeInfo.pTicker->setCallback([&sNodeInfo](uint64_t nContext)
		{
			if (sNodeInfo.pCoreConnectionToOtherNode != nullptr)
				return;

			if (sNodeInfo.sNodeBaseInfo.nPort == 0 || sNodeInfo.sNodeBaseInfo.szHost.empty())
				return;

			char szBuf[64] = { 0 };
			base::crt::snprintf(szBuf, _countof(szBuf), "%d", sNodeInfo.sNodeBaseInfo.nID);
			CBaseApp::Inst()->getBaseConnectionMgr()->connect(sNodeInfo.sNodeBaseInfo.szHost, sNodeInfo.sNodeBaseInfo.nPort, eBCT_ConnectionToOtherNode, szBuf, sNodeInfo.sNodeBaseInfo.nSendBufSize, sNodeInfo.sNodeBaseInfo.nRecvBufSize);
		});

		CBaseApp::Inst()->registerTicker(sNodeInfo.pTicker.get(), 0, _CHECK_CONNECT_TIME, 0);

		this->m_mapNodeName[sNodeBaseInfo.szName] = sNodeBaseInfo.nID;

		PrintInfo("add proxy node node_id: %d node_name: %s", sNodeBaseInfo.nID, sNodeBaseInfo.szName.c_str());
	}
	
	void CCoreOtherNodeProxy::delNodeBaseInfo(uint16_t nID)
	{
		auto iter = this->m_mapNodeInfo.find(nID);
		if (iter == this->m_mapNodeInfo.end())
			return;

		std::string szName = iter->second.sNodeBaseInfo.szName;
		this->m_mapNodeName.erase(iter->second.sNodeBaseInfo.szName);

		this->m_mapNodeInfo.erase(iter);

		PrintInfo("del proxy node node_id: %d node_name: %s", nID, szName.c_str());
	}

	uint16_t CCoreOtherNodeProxy::getNodeID(const std::string& szName) const
	{
		auto iter = this->m_mapNodeName.find(szName);
		if (iter == this->m_mapNodeName.end())
			return 0;

		return iter->second;
	}

	const SNodeBaseInfo* CCoreOtherNodeProxy::getNodeBaseInfo(uint16_t nID) const
	{
		auto iter = this->m_mapNodeInfo.find(nID);
		if (iter == this->m_mapNodeInfo.end())
			return nullptr;
		
		return &iter->second.sNodeBaseInfo;
	}

	bool CCoreOtherNodeProxy::addCoreConnectionToOtherNode(CCoreConnectionToOtherNode* pCoreConnectionToOtherNode)
	{
		DebugAstEx(pCoreConnectionToOtherNode != nullptr, false);

		auto iter = this->m_mapNodeInfo.find(pCoreConnectionToOtherNode->getNodeID());
		if (iter == this->m_mapNodeInfo.end())
		{
			PrintWarning("unknwon node node_id: %d remote_addr: %s %d", pCoreConnectionToOtherNode->getNodeID(), pCoreConnectionToOtherNode->getRemoteAddr().szHost, pCoreConnectionToOtherNode->getRemoteAddr().nPort);
			return false;
		}

		DebugAstEx(iter->second.pCoreConnectionToOtherNode == nullptr, false);

		iter->second.pCoreConnectionToOtherNode = pCoreConnectionToOtherNode;

		return true;
	}

	CCoreConnectionToOtherNode* CCoreOtherNodeProxy::getCoreConnectionToOtherNode(uint16_t nID) const
	{
		auto iter = this->m_mapNodeInfo.find(nID);
		if (iter == this->m_mapNodeInfo.end())
			return nullptr;

		return iter->second.pCoreConnectionToOtherNode;
	}

	void CCoreOtherNodeProxy::delCoreConnectionToOtherNode(uint16_t nID)
	{
		auto iter = this->m_mapNodeInfo.find(nID);
		if (iter == this->m_mapNodeInfo.end())
			return;

		iter->second.pCoreConnectionToOtherNode = nullptr;
	}

	bool CCoreOtherNodeProxy::addCoreConnectionFromOtherNode(uint16_t nID, CCoreConnectionFromOtherNode* pCoreConnectionFromOtherNode)
	{
		DebugAstEx(pCoreConnectionFromOtherNode != nullptr, false);

		auto iter = this->m_mapNodeInfo.find(nID);
		if (iter == this->m_mapNodeInfo.end())
		{
			PrintWarning("unknwon node node_id: %d remote_addr: %s %d", nID, pCoreConnectionFromOtherNode->getRemoteAddr().szHost, pCoreConnectionFromOtherNode->getRemoteAddr().nPort);
			return false;
		}

		DebugAstEx(iter->second.pCoreConnectionFromOtherNode == nullptr, false);

		iter->second.pCoreConnectionFromOtherNode = pCoreConnectionFromOtherNode;

		return true;
	}

	CCoreConnectionFromOtherNode* CCoreOtherNodeProxy::getCoreConnectionFromOtherNode(uint16_t nID) const
	{
		auto iter = this->m_mapNodeInfo.find(nID);
		if (iter == this->m_mapNodeInfo.end())
			return nullptr;

		return iter->second.pCoreConnectionFromOtherNode;
	}

	void CCoreOtherNodeProxy::delCoreConnectionFromOtherNode(uint16_t nID)
	{
		auto iter = this->m_mapNodeInfo.find(nID);
		if (iter == this->m_mapNodeInfo.end())
			return;

		iter->second.pCoreConnectionFromOtherNode = nullptr;
	}
}