#include "stdafx.h"
#include "global_service_registry_proxy.h"
#include "local_service_registry_proxy.h"
#include "base_connection_mgr.h"
#include "core_app.h"

#include "libBaseCommon/string_util.h"
#include "libBaseCommon/defer.h"

#define _CHECK_CONNECT_TIME 5000

namespace core
{
	CGlobalServiceRegistryProxy::CGlobalServiceRegistryProxy()
	{
		this->m_tickCheckConnectMaster.setCallback(std::bind(&CGlobalServiceRegistryProxy::onCheckConnectMaster, this, std::placeholders::_1));
	}

	CGlobalServiceRegistryProxy::~CGlobalServiceRegistryProxy()
	{
	}

	bool CGlobalServiceRegistryProxy::init(tinyxml2::XMLElement* pXMLElement)
	{
		DebugAstEx(pXMLElement != nullptr, false);

		tinyxml2::XMLElement* pConnectServiceInfoXML = pXMLElement->FirstChildElement("connect_service_info");
		if (pConnectServiceInfoXML != nullptr)
		{
			for (tinyxml2::XMLElement* pServiceInfoXML = pConnectServiceInfoXML->FirstChildElement("service_info"); pServiceInfoXML != nullptr; pServiceInfoXML = pServiceInfoXML->NextSiblingElement("service_info"))
			{
				std::string szServiceName = pServiceInfoXML->Attribute("service_name");
				std::string szServiceType = pServiceInfoXML->Attribute("service_type");

				if (szServiceName == "*")
					this->m_setConnectServiceType.insert(szServiceType);
				else
					this->m_setConnectServiceName.insert(szServiceName);
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

			CCoreApp::Inst()->registerTicker(CCoreApp::Inst()->getGlobalLogicMessageQueue(), &this->m_tickCheckConnectMaster, 1000, 5 * 1000, 0);
		}

		CCoreApp::Inst()->getGlobalBaseConnectionMgr()->addConnectFailCallback("master", std::bind(&CGlobalServiceRegistryProxy::onConnectRefuse, this, std::placeholders::_1));

		std::set<std::string> setType;
		const std::vector<SServiceBaseInfo>& vecServiceBaseInfo = CCoreApp::Inst()->getCoreServiceMgr()->getServiceBaseInfo();
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];

			this->m_mapServiceNameByID[sServiceBaseInfo.szName] = sServiceBaseInfo.nID;
			this->m_mapServiceIDByName[sServiceBaseInfo.nID] = sServiceBaseInfo.szName;
			
			SServiceIDInfo& sServiceIDInfo = this->m_mapServiceIDInfoByType[sServiceBaseInfo.szType];
			sServiceIDInfo.setServiceID.insert(sServiceBaseInfo.nID);
			sServiceIDInfo.setActiveServiceID.insert(sServiceBaseInfo.nID);

			setType.insert(sServiceBaseInfo.szType);
		}

		this->updateActiveServiceID(setType);

		return true;
	}

	void CGlobalServiceRegistryProxy::addNodeProxyInfo(const SNodeBaseInfo& sNodeBaseInfo, const std::vector<SServiceBaseInfo>& vecServiceBaseInfo, bool bExcludeConnect)
	{
		auto iter = this->m_mapNodeProxyInfo.find(sNodeBaseInfo.nID);
		if (iter != this->m_mapNodeProxyInfo.end())
			return;

		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			DebugAst(this->m_mapServiceNameByID.find(vecServiceBaseInfo[i].szName) == this->m_mapServiceNameByID.end());
			DebugAst(this->m_mapServiceIDByName.find(vecServiceBaseInfo[i].nID) == this->m_mapServiceIDByName.end());

			DebugAst(this->m_mapServiceProxyInfo.find(vecServiceBaseInfo[i].nID) == this->m_mapServiceProxyInfo.end());
		}
		
		SNodeProxyInfo& sNodeProxyInfo = this->m_mapNodeProxyInfo[sNodeBaseInfo.nID];

		sNodeProxyInfo.nSocketID = 0;
		sNodeProxyInfo.sNodeBaseInfo = sNodeBaseInfo;
		sNodeProxyInfo.vecServiceBaseInfo = vecServiceBaseInfo;

		std::set<std::string> setType;
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];

			SServiceProxyInfo sServiceProxyInfo;
			sServiceProxyInfo.sServiceBaseInfo = vecServiceBaseInfo[i];
			sServiceProxyInfo.nSocketID = 0;
			this->m_mapServiceProxyInfo[sServiceBaseInfo.nID] = sServiceProxyInfo;

			this->m_mapServiceNameByID[sServiceBaseInfo.szName] = sServiceBaseInfo.nID;
			this->m_mapServiceIDByName[sServiceBaseInfo.nID] = sServiceBaseInfo.szName;
			
			SServiceIDInfo& sServiceIDInfo = this->m_mapServiceIDInfoByType[sServiceBaseInfo.szType];
			sServiceIDInfo.setServiceID.insert(sServiceBaseInfo.nID);
			sServiceIDInfo.setActiveServiceID.insert(sServiceBaseInfo.nID);

			setType.insert(sServiceBaseInfo.szType);
		}

		this->updateActiveServiceID(setType);
		
		sNodeProxyInfo.pTicker = std::make_unique<CTicker>();
		// 不用担心sNodeProxyInfo的生命周期问题
		sNodeProxyInfo.pTicker->setCallback([&sNodeProxyInfo](uint64_t nContext)
		{
			if (sNodeProxyInfo.nSocketID != 0)
				return;

			CCoreApp::Inst()->getGlobalBaseConnectionMgr()->connect(sNodeProxyInfo.sNodeBaseInfo.szHost, sNodeProxyInfo.sNodeBaseInfo.nPort, "CBaseConnectionOtherNode", "", sNodeProxyInfo.sNodeBaseInfo.nSendBufSize, sNodeProxyInfo.sNodeBaseInfo.nRecvBufSize, nullptr);
		});

		bool bConnect = false;
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			if (this->m_setConnectServiceName.find(vecServiceBaseInfo[i].szName) != this->m_setConnectServiceName.end())
			{
				bConnect = true;
				break;
			}

			if (this->m_setConnectServiceType.find(vecServiceBaseInfo[i].szType) != this->m_setConnectServiceType.end())
			{
				bConnect = true;
				break;
			}
		}

		PrintInfo("add proxy node node_id: {} node_name: {} node_group: {}", sNodeBaseInfo.nID, sNodeBaseInfo.szName, sNodeBaseInfo.szGroup);

		if (bConnect && !bExcludeConnect)
		{
			DebugAst(!sNodeProxyInfo.sNodeBaseInfo.szHost.empty() && sNodeProxyInfo.sNodeBaseInfo.nPort != 0);

			CCoreApp::Inst()->registerTicker(CCoreApp::Inst()->getGlobalLogicMessageQueue(), sNodeProxyInfo.pTicker.get(), _CHECK_CONNECT_TIME, _CHECK_CONNECT_TIME, 0);
		}

		this->updateLocalServiceRegistryProxy();
	}
	
	void CGlobalServiceRegistryProxy::delNodeProxyInfo(uint32_t nNodeID)
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return;

		SNodeProxyInfo& sNodeProxyInfo = iter->second;

		std::string szName = sNodeProxyInfo.sNodeBaseInfo.szName;
		std::string szGroup = sNodeProxyInfo.sNodeBaseInfo.szGroup;

		if (sNodeProxyInfo.nSocketID != 0)
		{
			CBaseConnectionOtherNode* pBaseConnectionOtherNode = dynamic_cast<CBaseConnectionOtherNode*>(CCoreApp::Inst()->getGlobalBaseConnectionMgr()->getBaseConnectionBySocketID(sNodeProxyInfo.nSocketID));
			if (nullptr != pBaseConnectionOtherNode || !pBaseConnectionOtherNode->isClosing())
			{
				pBaseConnectionOtherNode->setNodeID(0);
				// 连接还活着，直接断开,在onDisconnect由于节点id被设置成0，所以不会重复发送服务断开通知事件
				pBaseConnectionOtherNode->shutdown(true, "del node");
				this->setOtherNodeSocketIDByNodeID(nNodeID, 0);
				this->onNodeDisconnect(nNodeID);
			}
		}
		
		std::set<std::string> setType;
		for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = sNodeProxyInfo.vecServiceBaseInfo[i];

			this->m_mapServiceNameByID.erase(sServiceBaseInfo.szName);
			this->m_mapServiceIDByName.erase(sServiceBaseInfo.nID);
			this->m_mapServiceProxyInfo.erase(sServiceBaseInfo.nID);

			SServiceIDInfo& sServiceIDInfo = this->m_mapServiceIDInfoByType[sServiceBaseInfo.szType];
			sServiceIDInfo.setServiceID.erase(sServiceBaseInfo.nID);
			sServiceIDInfo.setActiveServiceID.erase(sServiceBaseInfo.nID);
			
			setType.insert(sServiceBaseInfo.szType);
		}

		this->m_mapNodeProxyInfo.erase(iter);
		this->updateActiveServiceID(setType);
		
		PrintInfo("del proxy node node_id: {} node_name: {} node_group: {}", nNodeID, szName, szGroup);
	
		this->updateLocalServiceRegistryProxy();
	}

	void CGlobalServiceRegistryProxy::setNodeProxyInfo(const std::map<uint32_t, SNodeInfo>& mapNodeInfo, const std::set<uint32_t>& setExcludeConnectNodeID)
	{
		std::set<uint32_t> setDelNode;
		for (auto iter = this->m_mapNodeProxyInfo.begin(); iter != this->m_mapNodeProxyInfo.end(); ++iter)
		{
			uint32_t nNodeID = iter->first;
			if (mapNodeInfo.find(nNodeID) == mapNodeInfo.end())
				setDelNode.insert(nNodeID);
		}

		for (auto iter = setDelNode.begin(); iter != setDelNode.end(); ++iter)
		{
			this->delNodeProxyInfo(*iter);
		}

		for (auto iter = mapNodeInfo.begin(); iter != mapNodeInfo.end(); ++iter)
		{
			uint32_t nNodeID = iter->first;
			if (this->m_mapNodeProxyInfo.find(nNodeID) != this->m_mapNodeProxyInfo.end())
				continue;

			const SNodeInfo& sNodeInfo = iter->second;
			this->addNodeProxyInfo(sNodeInfo.sNodeBaseInfo, sNodeInfo.vecServiceBaseInfo, setExcludeConnectNodeID.find(sNodeInfo.sNodeBaseInfo.nID) != setExcludeConnectNodeID.end());
		}
	}

	bool CGlobalServiceRegistryProxy::getServiceBaseInfoByNodeID(uint32_t nNodeID, std::vector<SServiceBaseInfo>& vecServiceBaseInfo)
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
			return false;

		vecServiceBaseInfo = iter->second.vecServiceBaseInfo;
		return true;
	}

	void CGlobalServiceRegistryProxy::updateActiveServiceID(const std::set<std::string>& setType)
	{
		for (auto iter = setType.begin(); iter != setType.end(); ++iter)
		{
			SServiceIDInfo& sActiveServiceIDInfo = this->m_mapServiceIDInfoByType[*iter];
			sActiveServiceIDInfo.vecActiveServiceID.clear();
			for (auto iter = sActiveServiceIDInfo.setActiveServiceID.begin(); iter != sActiveServiceIDInfo.setActiveServiceID.end(); ++iter)
			{
				sActiveServiceIDInfo.vecActiveServiceID.push_back(*iter);
			}
		}
	}

	bool CGlobalServiceRegistryProxy::setOtherNodeSocketIDByNodeID(uint32_t nNodeID, uint64_t nSocketID)
	{
		auto iter = this->m_mapNodeProxyInfo.find(nNodeID);
		if (iter == this->m_mapNodeProxyInfo.end())
		{
			PrintWarning("CServiceRegistryProxy::setOtherNodeSocketIDByNodeID unknwon node node_id: {} socket_id: {}", nNodeID, nSocketID);
			return false;
		}

		SNodeProxyInfo& sNodeProxyInfo = iter->second;
		if (nSocketID != 0)
		{
			if (sNodeProxyInfo.nSocketID != 0)
			{
				PrintWarning("CServiceRegistryProxy::setOtherNodeSocketIDByNodeID dup node connection node_id: {} socket_id: {}", nNodeID, nSocketID);
				return false;
			}

			std::set<std::string> setType;
			for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
			{
				auto iter = this->m_mapServiceProxyInfo.find(sNodeProxyInfo.vecServiceBaseInfo[i].nID);
				if (iter == this->m_mapServiceProxyInfo.end())
					continue;

				SServiceProxyInfo& sServiceProxyInfo = iter->second;
				SServiceBaseInfo& sServiceBaseInfo = sServiceProxyInfo.sServiceBaseInfo;
				sServiceProxyInfo.nSocketID = nSocketID;

				SServiceIDInfo& sServiceIDInfo = this->m_mapServiceIDInfoByType[sServiceBaseInfo.szType];
				sServiceIDInfo.setActiveServiceID.insert(sServiceBaseInfo.nID);
				
				setType.insert(sServiceBaseInfo.szType);
			}

			sNodeProxyInfo.nSocketID = nSocketID;

			this->updateActiveServiceID(setType);
			
			PrintInfo("other node connect node_id: {} node_name: {} node_group: {} socket_id: {}", nNodeID, sNodeProxyInfo.sNodeBaseInfo.szName, sNodeProxyInfo.sNodeBaseInfo.szGroup, nSocketID);
		}
		else
		{
			std::set<std::string> setType;
			for (size_t i = 0; i < sNodeProxyInfo.vecServiceBaseInfo.size(); ++i)
			{
				auto iter = this->m_mapServiceProxyInfo.find(sNodeProxyInfo.vecServiceBaseInfo[i].nID);
				if (iter == this->m_mapServiceProxyInfo.end())
					continue;

				SServiceProxyInfo& sServiceProxyInfo = iter->second;
				SServiceBaseInfo& sServiceBaseInfo = sServiceProxyInfo.sServiceBaseInfo;

				sServiceProxyInfo.nSocketID = 0;

				SServiceIDInfo& sServiceIDInfo = this->m_mapServiceIDInfoByType[sServiceBaseInfo.szType];
				sServiceIDInfo.setActiveServiceID.erase(sServiceBaseInfo.nID);
				
				setType.insert(sServiceBaseInfo.szType);
			}

			uint64_t nSocketID = sNodeProxyInfo.nSocketID;
			sNodeProxyInfo.nSocketID = 0;
			
			this->updateActiveServiceID(setType);
			
			PrintInfo("other node disconnect node_id: {} node_name: {} node_group: {} socket_id: {}", nNodeID, sNodeProxyInfo.sNodeBaseInfo.szName, sNodeProxyInfo.sNodeBaseInfo.szGroup, nSocketID);
		}

		this->updateLocalServiceRegistryProxy();

		return true;
	}

	bool CGlobalServiceRegistryProxy::addBaseConnectionToMaster(CBaseConnectionToMaster* pBaseConnectionToMaster)
	{
		DebugAstEx(pBaseConnectionToMaster != nullptr, false);
		auto iter = this->m_mapMasterInfo.find(pBaseConnectionToMaster->getMasterID());
		DebugAstEx(iter != this->m_mapMasterInfo.end(), false);
		DebugAstEx(iter->second.pBaseConnectionToMaster == nullptr, false);

		iter->second.pBaseConnectionToMaster = pBaseConnectionToMaster;
		
		PrintInfo("add master master_id: {}", pBaseConnectionToMaster->getMasterID());

		return true;
	}

	void CGlobalServiceRegistryProxy::delBaseConnectionToMaster(uint32_t nMasterID)
	{
		auto iter = this->m_mapMasterInfo.find(nMasterID);
		DebugAst(iter != this->m_mapMasterInfo.end());

		iter->second.pBaseConnectionToMaster = nullptr;
		iter->second.bActive = false;

		PrintInfo("del master master_id: {}", nMasterID);
	}

	void CGlobalServiceRegistryProxy::onCheckConnectMaster(uint64_t nContext)
	{
		for (auto iter = this->m_mapMasterInfo.begin(); iter != this->m_mapMasterInfo.end(); ++iter)
		{
			SMasterInfo& sMasterInfo = iter->second;
			if (sMasterInfo.bActive)
				continue;

			sMasterInfo.bActive = true;
			char szBuf[256] = { 0 };
			base::function_util::snprintf(szBuf, _countof(szBuf), "master%d", sMasterInfo.nID);
			CCoreApp::Inst()->getGlobalBaseConnectionMgr()->connect(sMasterInfo.szHost, sMasterInfo.nPort, "CBaseConnectionToMaster", szBuf, 10 * 1024, 10 * 1024, nullptr);;
		}
	}

	void CGlobalServiceRegistryProxy::onConnectRefuse(const std::string& szContext)
	{
		if (szContext.find("master") == std::string::npos)
			return;

		uint32_t nID = 0;
		base::string_util::convert_to_value(szContext.c_str() + std::string("master").size(), nID);

		auto iter = this->m_mapMasterInfo.find(nID);
		DebugAst(iter != this->m_mapMasterInfo.end());

		iter->second.bActive = false;
	}

	void CGlobalServiceRegistryProxy::onNodeConnect(uint32_t nNodeID)
	{
		const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService();
		std::vector<SServiceBaseInfo> vecServiceBaseInfo;
		this->getServiceBaseInfoByNodeID(nNodeID, vecServiceBaseInfo);
		for (size_t i = 0; i < vecCoreService.size(); ++i)
		{
			CCoreService* pCoreService = vecCoreService[i];
			for (size_t j = 0; j < vecServiceBaseInfo.size(); ++j)
			{
				SMCT_SERVICE_CONNECT* pContext = new SMCT_SERVICE_CONNECT();
				pContext->szType = vecServiceBaseInfo[j].szType;
				pContext->nID = vecServiceBaseInfo[j].nID;

				SMessagePacket sMessagePacket;
				sMessagePacket.nType = eMCT_SERVICE_CONNECT;
				sMessagePacket.pData = pContext;
				sMessagePacket.nDataSize = 0;

				pCoreService->getMessageQueue()->send(sMessagePacket);
			}
		}
	}

	void CGlobalServiceRegistryProxy::onNodeDisconnect(uint32_t nNodeID)
	{
		const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService();
		std::vector<SServiceBaseInfo> vecServiceBaseInfo;
		this->getServiceBaseInfoByNodeID(nNodeID, vecServiceBaseInfo);
		for (size_t i = 0; i < vecCoreService.size(); ++i)
		{
			CCoreService* pCoreService = vecCoreService[i];
			for (size_t j = 0; j < vecServiceBaseInfo.size(); ++j)
			{
				SMCT_SERVICE_DISCONNECT* pContext = new SMCT_SERVICE_DISCONNECT();
				pContext->szType = vecServiceBaseInfo[j].szType;
				pContext->nID = vecServiceBaseInfo[j].nID;

				SMessagePacket sMessagePacket;
				sMessagePacket.nType = eMCT_SERVICE_DISCONNECT;
				sMessagePacket.pData = pContext;
				sMessagePacket.nDataSize = 0;

				pCoreService->getMessageQueue()->send(sMessagePacket);
			}
		}
	}

	const std::set<std::string>& CGlobalServiceRegistryProxy::getConnectServiceName() const
	{
		return this->m_setConnectServiceName;
	}

	const std::set<std::string>& CGlobalServiceRegistryProxy::getConnectServiceType() const
	{
		return this->m_setConnectServiceType;
	}

	void CGlobalServiceRegistryProxy::fillLocalServiceRegistryProxy(CLocalServiceRegistryProxy* pLocalServiceRegistryProxy)
	{
		DebugAst(pLocalServiceRegistryProxy != nullptr);

		pLocalServiceRegistryProxy->m_mapServiceProxyInfo = this->m_mapServiceProxyInfo;
		pLocalServiceRegistryProxy->m_mapServiceIDInfoByType = this->m_mapServiceIDInfoByType;
		pLocalServiceRegistryProxy->m_mapServiceNameByID = this->m_mapServiceNameByID;
		pLocalServiceRegistryProxy->m_mapServiceIDByName = this->m_mapServiceIDByName;
	}

	void CGlobalServiceRegistryProxy::updateLocalServiceRegistryProxy()
	{
		const std::vector<CCoreService*>& vecCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService();
		for (size_t i = 0; i < vecCoreService.size(); ++i)
		{
			CLocalServiceRegistryProxy* pLocalServiceRegistryProxy = new CLocalServiceRegistryProxy();
			SMCT_UPDATE_SERVICE_REGISTRY_PROXY* pContext = new SMCT_UPDATE_SERVICE_REGISTRY_PROXY();
			pContext->pProxy = pLocalServiceRegistryProxy;

			this->fillLocalServiceRegistryProxy(pLocalServiceRegistryProxy);

			SMessagePacket sMessagePacket;
			sMessagePacket.nType = eMCT_UPDATE_SERVICE_REGISTRY_PROXY;
			sMessagePacket.pData = pContext;
			sMessagePacket.nDataSize = 0;

			vecCoreService[i]->getMessageQueue()->send(sMessagePacket);
		}
	}
}