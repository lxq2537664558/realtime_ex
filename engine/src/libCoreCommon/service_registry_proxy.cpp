#include "stdafx.h"
#include "service_registry_proxy.h"
#include "base_connection_mgr.h"
#include "core_app.h"

#include "libBaseCommon/string_util.h"

#define _CHECK_CONNECT_TIME 5000

namespace core
{
	CServiceRegistryProxy::CServiceRegistryProxy()
	{
		this->m_tickCheckConnectMaster.setCallback(std::bind(&CServiceRegistryProxy::onCheckConnectMaster, this, std::placeholders::_1));
	}

	CServiceRegistryProxy::~CServiceRegistryProxy()
	{
	}

	bool CServiceRegistryProxy::init(tinyxml2::XMLElement* pXMLElement)
	{
		DebugAstEx(pXMLElement != nullptr, false);

		tinyxml2::XMLElement* pConnectServiceInfoXML = pXMLElement->FirstChildElement("connect_service_info");
		if (pConnectServiceInfoXML != nullptr)
		{
			uint32_t nGlobalServiceInvokeTimeout = pConnectServiceInfoXML->UnsignedAttribute("invoke_timeout");
			if (nGlobalServiceInvokeTimeout == 0)
			{
				PrintWarning("global invoke timeout errror");
				return false;
			}

			for (tinyxml2::XMLElement* pServiceInfoXML = pConnectServiceInfoXML->FirstChildElement("service_info"); pServiceInfoXML != nullptr; pServiceInfoXML = pServiceInfoXML->NextSiblingElement("service_info"))
			{
				std::string szServiceName = pServiceInfoXML->Attribute("service_name");
				std::string szServiceType = pServiceInfoXML->Attribute("service_type");
				uint32_t nServiceInvokeTimeout = pServiceInfoXML->UnsignedAttribute("invoke_timeout");
				if (nServiceInvokeTimeout == 0)
					nServiceInvokeTimeout = nGlobalServiceInvokeTimeout;

				if (szServiceName == "*")
					this->m_mapConnectServiceType[szServiceType] = nServiceInvokeTimeout;
				else
					this->m_mapConnectServiceName[szServiceName] = nServiceInvokeTimeout;
			}
		}

		tinyxml2::XMLElement* pMonsterInfoXML = pXMLElement->FirstChildElement("monster_info");
		if (pMonsterInfoXML != nullptr)
		{
			for (tinyxml2::XMLElement* pMonsterXML = pMonsterInfoXML->FirstChildElement("master"); pMonsterXML != nullptr; pMonsterXML = pMonsterXML->NextSiblingElement("master"))
			{
				SMasterInfo sMasterInfo;
				sMasterInfo.nID = pMonsterXML->UnsignedAttribute("id");
				sMasterInfo.szHost = pMonsterXML->Attribute("host");
				sMasterInfo.nPort = (uint16_t)pMonsterXML->UnsignedAttribute("port");
				sMasterInfo.pBaseConnectionToMaster = nullptr;
				sMasterInfo.bActive = false;
				this->m_mapMasterInfo[sMasterInfo.nID] = sMasterInfo;
			}

			CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, 0, 0, &this->m_tickCheckConnectMaster, 5 * 1000, 5 * 1000, 0);
		}

		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgr()->addConnectFailCallback("master", std::bind(&CServiceRegistryProxy::onConnectRefuse, this, std::placeholders::_1));

		std::vector<SServiceBaseInfo> vecServiceBaseInfo = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getServiceBaseInfo();
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];
			this->m_mapServiceIDByServiceType[sServiceBaseInfo.szType].push_back(sServiceBaseInfo.nID);
			this->m_mapServiceNameByID[sServiceBaseInfo.szName] = sServiceBaseInfo.nID;
			this->m_mapServiceIDByName[sServiceBaseInfo.nID] = sServiceBaseInfo.szName;
		}

		return true;
	}

	uint32_t CServiceRegistryProxy::getServiceInvokeTimeout(uint32_t nServiceID) const
	{
		auto iterService = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iterService == this->m_mapServiceProxyInfo.end())
			return 0;

		const std::string& szName = iterService->second.sServiceBaseInfo.szName;
		const std::string& szType = iterService->second.sServiceBaseInfo.szType;

		auto iter = this->m_mapConnectServiceName.find(szName);
		if (iter != this->m_mapConnectServiceName.end())
			return iter->second;

		auto iterType = this->m_mapConnectServiceType.find(szType);
		if (iterType != this->m_mapConnectServiceType.end())
			return iterType->second;
		
		return 0;
	}

	void CServiceRegistryProxy::addNodeProxyInfo(const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo, bool bMaster)
	{
		auto iter = this->m_mapNodeProxyInfo.find(sNodeBaseInfo.nID);
		if (iter != this->m_mapNodeProxyInfo.end())
			return;

		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			DebugAst(this->m_mapServiceNameByID.find(vecServiceBaseInfo[i].szName) == this->m_mapServiceNameByID.end());
			DebugAst(this->m_mapServiceIDByName.find(vecServiceBaseInfo[i].nID) == this->m_mapServiceIDByName.end());
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

			CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgr()->connect(sNodeProxyInfo.sNodeBaseInfo.szHost, sNodeProxyInfo.sNodeBaseInfo.nPort, "CBaseConnectionOtherNode", "", sNodeProxyInfo.sNodeBaseInfo.nSendBufSize, sNodeProxyInfo.sNodeBaseInfo.nRecvBufSize, nullptr);
		});

		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];

			SServiceProxyInfo sServiceProxyInfo;
			sServiceProxyInfo.sServiceBaseInfo = vecServiceBaseInfo[i];
			sServiceProxyInfo.pBaseConnectionOtherNode = nullptr;
			this->m_mapServiceProxyInfo[sServiceBaseInfo.nID] = sServiceProxyInfo;

			this->m_mapServiceNameByID[sServiceBaseInfo.szName] = sServiceBaseInfo.nID;
			this->m_mapServiceIDByName[sServiceBaseInfo.nID] = sServiceBaseInfo.szName;
			this->m_mapServiceIDByServiceType[sServiceBaseInfo.szType].push_back(sServiceBaseInfo.nID);
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

		if (bConnect)
			CCoreApp::Inst()->registerTicker(CTicker::eTT_Service, 0, 0, sNodeProxyInfo.pTicker.get(), _CHECK_CONNECT_TIME, _CHECK_CONNECT_TIME, 0);

		PrintInfo("add proxy node node_id: {} node_name: {}", sNodeBaseInfo.nID, sNodeBaseInfo.szName);
	}
	
	void CServiceRegistryProxy::delNodeProxyInfo(uint32_t nID)
	{
		auto iter = this->m_mapNodeProxyInfo.find(nID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return;

		SNodeProxyInfo& sNodeProxyInfo = iter->second;

		std::string szName = sNodeProxyInfo.sNodeBaseInfo.szName;
		
		if (sNodeProxyInfo.pBaseConnectionOtherNode != nullptr)
			sNodeProxyInfo.pBaseConnectionOtherNode->shutdown(true, "del node");

		for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
		{
			this->m_mapServiceNameByID.erase(sNodeProxyInfo.vecServiceBaseInfo[i].szName);
			this->m_mapServiceProxyInfo.erase(sNodeProxyInfo.vecServiceBaseInfo[i].nID);
		}

		for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = sNodeProxyInfo.vecServiceBaseInfo[i];
			std::vector<uint32_t>& vecServiceID = this->m_mapServiceIDByServiceType[sServiceBaseInfo.szType];
			for (size_t j = 0; j < vecServiceID.size(); ++j)
			{
				if (vecServiceID[j] == sServiceBaseInfo.nID)
				{
					vecServiceID.erase(vecServiceID.begin() + j);
				}
			}
		}

		this->m_mapNodeProxyInfo.erase(iter);
		
		PrintInfo("del proxy node node_id: {} node_name: {}", nID, szName);
	}

	uint32_t CServiceRegistryProxy::getServiceID(const std::string& szName) const
	{
		auto iter = this->m_mapServiceNameByID.find(szName);
		if (iter == this->m_mapServiceNameByID.end())
			return 0;

		return iter->second;
	}

	std::string CServiceRegistryProxy::getServiceType(uint32_t nServiceID) const
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
		{
			CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreService(nServiceID);
			if (pCoreService == nullptr)
				return "";

			return pCoreService->getServiceBaseInfo().szType;
		}

		return iter->second.sServiceBaseInfo.szType;
	}

	std::string CServiceRegistryProxy::getServiceName(uint32_t nServiceID) const
	{
		auto iter = this->m_mapServiceIDByName.find(nServiceID);
		if (iter == this->m_mapServiceIDByName.end())
			return "";

		return iter->second;
	}

	const std::vector<uint32_t>& CServiceRegistryProxy::getServiceIDByTypeName(const std::string& szName) const
	{
		auto iter = this->m_mapServiceIDByServiceType.find(szName);
		if (iter == this->m_mapServiceIDByServiceType.end())
		{
			static std::vector<uint32_t> vecEmpty;
			return vecEmpty;
		}
		
		return iter->second;
	}

	const SServiceBaseInfo* CServiceRegistryProxy::getServiceBaseInfoByServiceID(uint32_t nServiceID) const
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
			return nullptr;
		
		return &iter->second.sServiceBaseInfo;
	}

	bool CServiceRegistryProxy::getServiceBaseInfoByNodeID(uint32_t nNodeID, std::vector<SServiceBaseInfo>& vecServiceBaseInfo) const
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return false;

		vecServiceBaseInfo = iter->second.vecServiceBaseInfo;
		return true;
	}

	void CServiceRegistryProxy::delBaseConnectionOtherNodeByNodeID(uint32_t nNodeID)
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

		PrintInfo("other node disconnect node_id: {} node_name: {}", nNodeID, sNodeProxyInfo.sNodeBaseInfo.szName);
	}

	bool CServiceRegistryProxy::addBaseConnectionOtherNodeByNodeID(uint32_t nNodeID, CBaseConnectionOtherNode* pBaseConnectionOtherNode)
	{
		DebugAstEx(pBaseConnectionOtherNode != nullptr, false);

		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
		{
			PrintWarning("CServiceRegistryProxy::addBaseConnectionOtherNodeByNodeID unknwon node node_id: {} remote_addr: {} {}", nNodeID, pBaseConnectionOtherNode->getRemoteAddr().szHost, pBaseConnectionOtherNode->getRemoteAddr().nPort);
			return false;
		}

		SNodeProxyInfo& sNodeProxyInfo = iter->second;
		if (sNodeProxyInfo.pBaseConnectionOtherNode != nullptr)
		{
			PrintWarning("CServiceRegistryProxy::addBaseConnectionOtherNodeByNodeID dup node connection node_id: {} remote_addr: {} {}", nNodeID, pBaseConnectionOtherNode->getRemoteAddr().szHost, pBaseConnectionOtherNode->getRemoteAddr().nPort);
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

		PrintInfo("other node connect node_id: {} node_name: {}", nNodeID, sNodeProxyInfo.sNodeBaseInfo.szName);
		return true;
	}

	CBaseConnectionOtherNode* CServiceRegistryProxy::getBaseConnectionOtherNodeByNodeID(uint32_t nNodeID) const
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return nullptr;

		return iter->second.pBaseConnectionOtherNode;
	}

	CBaseConnectionOtherNode* CServiceRegistryProxy::getBaseConnectionOtherNodeByServiceID(uint32_t nServiceID) const
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
			return nullptr;

		return iter->second.pBaseConnectionOtherNode;
	}

	bool CServiceRegistryProxy::addBaseConnectionToMaster(CBaseConnectionToMaster* pBaseConnectionToMaster)
	{
		DebugAstEx(pBaseConnectionToMaster != nullptr, false);
		auto iter = this->m_mapMasterInfo.find(pBaseConnectionToMaster->getMasterID());
		DebugAstEx(iter != this->m_mapMasterInfo.end(), false);
		DebugAstEx(iter->second.pBaseConnectionToMaster == nullptr, false);

		iter->second.pBaseConnectionToMaster = pBaseConnectionToMaster;
		
		PrintInfo("add master master_id: {}", pBaseConnectionToMaster->getMasterID());

		return true;
	}

	void CServiceRegistryProxy::delBaseConnectionToMaster(uint32_t nMasterID)
	{
		auto iter = this->m_mapMasterInfo.find(nMasterID);
		DebugAst(iter != this->m_mapMasterInfo.end());

		iter->second.pBaseConnectionToMaster = nullptr;
		iter->second.bActive = false;

		PrintInfo("del master master_id: {}", nMasterID);
	}

	void CServiceRegistryProxy::onCheckConnectMaster(uint64_t nContext)
	{
		for (auto iter = this->m_mapMasterInfo.begin(); iter != this->m_mapMasterInfo.end(); ++iter)
		{
			SMasterInfo& sMasterInfo = iter->second;
			if (sMasterInfo.bActive)
				continue;

			sMasterInfo.bActive = true;
			char szBuf[256] = { 0 };
			base::function_util::snprintf(szBuf, _countof(szBuf), "master%d", sMasterInfo.nID);
			CBaseApp::Inst()->getBaseConnectionMgr()->connect(sMasterInfo.szHost, sMasterInfo.nPort, "CBaseConnectionToMaster", szBuf, 10 * 1024, 10 * 1024, nullptr);;
		}
	}

	void CServiceRegistryProxy::onConnectRefuse(const std::string& szContext)
	{
		if (szContext.find("master") == std::string::npos)
			return;

		uint32_t nID = 0;
		base::string_util::convert_to_value(szContext.c_str() + std::string("master").size(), nID);

		auto iter = this->m_mapMasterInfo.find(nID);
		DebugAst(iter != this->m_mapMasterInfo.end());

		iter->second.bActive = false;
	}

	bool CServiceRegistryProxy::isValidService(uint32_t nServiceID) const
	{
		return this->m_mapServiceIDByName.find(nServiceID) != this->m_mapServiceIDByName.end();
	}
}