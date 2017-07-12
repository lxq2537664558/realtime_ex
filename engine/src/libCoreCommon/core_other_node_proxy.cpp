#include "stdafx.h"
#include "core_other_node_proxy.h"
#include "base_connection_mgr.h"
#include "core_app.h"

#define _CHECK_CONNECT_TIME 5000

namespace core
{
	CCoreOtherNodeProxy::CCoreOtherNodeProxy()
	{

	}

	CCoreOtherNodeProxy::~CCoreOtherNodeProxy()
	{
	}

	bool CCoreOtherNodeProxy::init(tinyxml2::XMLElement* pXMLElement)
	{
		if (pXMLElement != nullptr)
		{
			for (tinyxml2::XMLElement* pServiceInfoXML = pXMLElement->FirstChildElement("service_info"); pServiceInfoXML != nullptr; pServiceInfoXML = pServiceInfoXML->NextSiblingElement("service_info"))
			{
				std::string szServiceName = pServiceInfoXML->Attribute("service_name");
				std::string szServiceType = pServiceInfoXML->Attribute("service_type");
				uint32_t nServiceInvokeTimeout = pServiceInfoXML->UnsignedAttribute("invoke_timeout");
				if (szServiceName == "*")
					this->m_mapConnectServiceType[szServiceType] = nServiceInvokeTimeout;
				else
					this->m_mapConnectServiceName[szServiceName] = nServiceInvokeTimeout;
			}
		}

		return true;
	}

	void CCoreOtherNodeProxy::addNodeProxyInfo(const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo, bool bMaster)
	{
		auto iter = this->m_mapNodeProxyInfo.find(sNodeBaseInfo.nID);
		if (iter != this->m_mapNodeProxyInfo.end())
			return;

		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			DebugAst(this->m_mapServiceName.find(vecServiceBaseInfo[i].szName) == this->m_mapServiceName.end());
			DebugAst(this->m_mapServiceProxyInfo.find(vecServiceBaseInfo[i].nID) == this->m_mapServiceProxyInfo.end());
		}
		
		SNodeProxyInfo& sNodeProxyInfo = this->m_mapNodeProxyInfo[sNodeBaseInfo.nID];
		sNodeProxyInfo.pBaseConnectionOtherNode = nullptr;
		sNodeProxyInfo.sNodeBaseInfo = sNodeBaseInfo;
		sNodeProxyInfo.vecServiceBaseInfo = vecServiceBaseInfo;
		
		sNodeProxyInfo.pTicker = std::make_unique<CTicker>();
		// 不用担心sNodeProxyInfo的生命周期问题
		sNodeProxyInfo.pTicker->setCallback([&sNodeProxyInfo](uint64_t nContext)
		{
			if (sNodeProxyInfo.pBaseConnectionOtherNode != nullptr)
				return;

			if (sNodeProxyInfo.sNodeBaseInfo.nPort == 0 || sNodeProxyInfo.sNodeBaseInfo.szHost.empty())
				return;

			char szBuf[64] = { 0 };
			base::crt::snprintf(szBuf, _countof(szBuf), "%d", sNodeProxyInfo.sNodeBaseInfo.nID);
			CCoreApp::Inst()->getBaseConnectionMgr()->connect(sNodeProxyInfo.sNodeBaseInfo.szHost, sNodeProxyInfo.sNodeBaseInfo.nPort, eBCT_ConnectionToOtherNode, szBuf, sNodeProxyInfo.sNodeBaseInfo.nSendBufSize, sNodeProxyInfo.sNodeBaseInfo.nRecvBufSize, nullptr);
		});

		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			this->m_mapServiceName[vecServiceBaseInfo[i].szName] = vecServiceBaseInfo[i].nID;
			this->m_mapServiceProxyInfo[vecServiceBaseInfo[i].nID] = { vecServiceBaseInfo[i], nullptr };
		}

		bool bConnect = false;
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			if (this->m_mapConnectServiceName.find(vecServiceBaseInfo[i].szName) != this->m_mapConnectServiceName.end())
			{
				bConnect = true;
				break;
			}

			if (this->m_mapConnectServiceType.find(vecServiceBaseInfo[i].szType) != this->m_mapConnectServiceType.end())
			{
				bConnect = true;
				break;
			}
		}

		if (bConnect && bMaster)
			CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, 0, sNodeProxyInfo.pTicker.get(), _CHECK_CONNECT_TIME, _CHECK_CONNECT_TIME, 0);

		PrintInfo("add proxy node node_id: %d node_name: %s", sNodeBaseInfo.nID, sNodeBaseInfo.szName.c_str());
	}
	
	void CCoreOtherNodeProxy::delNodeProxyInfo(uint16_t nID)
	{
		auto iter = this->m_mapNodeProxyInfo.find(nID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return;

		SNodeProxyInfo& sNodeProxyInfo = iter->second;

		std::string szName = sNodeProxyInfo.sNodeBaseInfo.szName;
		
		if (sNodeProxyInfo.pBaseConnectionOtherNode != nullptr)
			sNodeProxyInfo.pBaseConnectionOtherNode->shutdown(base::eNCCT_Force, "del node");

		for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
		{
			this->m_mapServiceName.erase(sNodeProxyInfo.vecServiceBaseInfo[i].szName);
			this->m_mapServiceProxyInfo.erase(sNodeProxyInfo.vecServiceBaseInfo[i].nID);
		}

		this->m_mapNodeProxyInfo.erase(iter);
		PrintInfo("del proxy node node_id: %d node_name: %s", nID, szName.c_str());
	}

	uint16_t CCoreOtherNodeProxy::getServiceID(const std::string& szName) const
	{
		auto iter = this->m_mapServiceName.find(szName);
		if (iter == this->m_mapServiceName.end())
			return 0;

		return iter->second;
	}

	const SServiceBaseInfo* CCoreOtherNodeProxy::getServiceBaseInfoByServiceID(uint16_t nServiceID) const
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
			return nullptr;
		
		return &iter->second.sServiceBaseInfo;
	}

	bool CCoreOtherNodeProxy::getServiceBaseInfoByNodeID(uint16_t nNodeID, std::vector<SServiceBaseInfo>& vecServiceBaseInfo) const
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return false;

		vecServiceBaseInfo = iter->second.vecServiceBaseInfo;
		return true;
	}

	void CCoreOtherNodeProxy::delBaseConnectionOtherNodeByNodeID(uint16_t nNodeID)
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return;

		SNodeProxyInfo& sNodeProxyInfo = iter->second;
		for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
		{
			auto iter = this->m_mapServiceProxyInfo.find(sNodeProxyInfo.vecServiceBaseInfo[i].nID);
			if (iter == this->m_mapServiceProxyInfo.end())
				continue;

			SServiceProxyInfo& sServiceProxyInfo = iter->second;
			sServiceProxyInfo.pBaseConnectionOtherNode = nullptr;
		}

		sNodeProxyInfo.pBaseConnectionOtherNode = nullptr;
	}

	bool CCoreOtherNodeProxy::addBaseConnectionOtherNodeByNodeID(uint16_t nNodeID, CBaseConnectionOtherNode* pBaseConnectionOtherNode)
	{
		DebugAstEx(pBaseConnectionOtherNode != nullptr, false);

		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
		{
			PrintWarning("unknwon node node_id: %d remote_addr: %s %d", nNodeID, pBaseConnectionOtherNode->getRemoteAddr().szHost, pBaseConnectionOtherNode->getRemoteAddr().nPort);
			return false;
		}

		SNodeProxyInfo& sNodeProxyInfo = iter->second;
		if (sNodeProxyInfo.pBaseConnectionOtherNode != nullptr)
		{
			PrintWarning("dup node connection node_id: %d remote_addr: %s %d", nNodeID, pBaseConnectionOtherNode->getRemoteAddr().szHost, pBaseConnectionOtherNode->getRemoteAddr().nPort);
			return false;
		}

		for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
		{
			auto iter = this->m_mapServiceProxyInfo.find(sNodeProxyInfo.vecServiceBaseInfo[i].nID);
			if (iter == this->m_mapServiceProxyInfo.end())
				continue;

			SServiceProxyInfo& sServiceProxyInfo = iter->second;
			sServiceProxyInfo.pBaseConnectionOtherNode = pBaseConnectionOtherNode;
		}

		sNodeProxyInfo.pBaseConnectionOtherNode = pBaseConnectionOtherNode;

		return true;
	}

	CBaseConnectionOtherNode* CCoreOtherNodeProxy::getBaseConnectionOtherNodeByNodeID(uint16_t nNodeID) const
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return nullptr;

		return iter->second.pBaseConnectionOtherNode;
	}

	CBaseConnectionOtherNode* CCoreOtherNodeProxy::getBaseConnectionOtherNodeByServiceID(uint16_t nServiceID) const
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
			return nullptr;

		return iter->second.pBaseConnectionOtherNode;
	}
}