#include "stdafx.h"
#include "core_service_proxy.h"
#include "core_service_kit_impl.h"

namespace core
{
#define _DEFAULT_INVOKE_TIMEOUT 10000

	CCoreServiceProxy::CCoreServiceProxy()
		: m_nProxyInvokTimeout(_DEFAULT_INVOKE_TIMEOUT)
		, m_nProxyRetries(0)
		, m_bProxyAny(false)
	{

	}

	CCoreServiceProxy::~CCoreServiceProxy()
	{

	}

	bool CCoreServiceProxy::init(tinyxml2::XMLElement* pRootXML)
	{
		DebugAstEx(pRootXML != nullptr, false);

		tinyxml2::XMLElement* pServiceProxyXML = pRootXML->FirstChildElement("service_proxy");
		if (pServiceProxyXML == nullptr)
			return true;

		this->m_nProxyInvokTimeout = pServiceProxyXML->UnsignedAttribute("timeout");
		this->m_szProxyLoadBalance = pServiceProxyXML->Attribute("loadbalance");
		this->m_szProxyFail = pServiceProxyXML->Attribute("fail");
		this->m_nProxyRetries = pServiceProxyXML->UnsignedAttribute("retries");

		for (tinyxml2::XMLElement* pMessageProxyXML = pServiceProxyXML->FirstChildElement("message_proxy"); pMessageProxyXML != nullptr; pMessageProxyXML = pMessageProxyXML->NextSiblingElement("message_proxy"))
		{
			SMetaMessageProxyInfo sMetaMessageProxyInfo;
			sMetaMessageProxyInfo.szMessageName = pMessageProxyXML->Attribute("name");
			sMetaMessageProxyInfo.nTimeout = pMessageProxyXML->UnsignedAttribute("timeout");
			sMetaMessageProxyInfo.szLoadbalanceName = pMessageProxyXML->Attribute("loadbalance");
			sMetaMessageProxyInfo.szFail = pMessageProxyXML->Attribute("fail");
			sMetaMessageProxyInfo.nRetries = pMessageProxyXML->UnsignedAttribute("retries");

			if (sMetaMessageProxyInfo.szMessageName.empty())
			{
				PrintWarning("empty message name");
				return false;
			}
			uint32_t nMessageID = base::hash(sMetaMessageProxyInfo.szMessageName.c_str());
			auto iter = this->m_mapMetaMessageProxyInfo.find(nMessageID);
			if (iter != this->m_mapMetaMessageProxyInfo.end())
			{
				PrintWarning("dup message name message_id: %d message_name: [%s, %s]", nMessageID, iter->second.szMessageName.c_str(), sMetaMessageProxyInfo.szMessageName.c_str());
				return false;
			}

			if (sMetaMessageProxyInfo.szMessageName == "*")
			{
				this->m_bProxyAny = true;
				this->m_sAnyMetaMessageProxyInfo = sMetaMessageProxyInfo;
			}
			
			this->m_mapMetaMessageProxyInfo[nMessageID] = sMetaMessageProxyInfo;
		}

		return true;
	}

	void CCoreServiceProxy::addMessageProxyInfo(const SMessageProxyInfo& sMessageProxyInfo)
	{
		uint32_t nMessageID =_GET_MESSAGE_ID(sMessageProxyInfo.szMessageName);
		if (this->m_mapMetaMessageProxyInfo.find(nMessageID) == this->m_mapMetaMessageProxyInfo.end() && !this->m_bProxyAny)
			return;

		auto iterName = this->m_mapMessageName.find(nMessageID);
		if (iterName != this->m_mapMessageName.end() && iterName->second != sMessageProxyInfo.szMessageName)
		{
			PrintWarning("dup message id service_name: %s message_id: %d old_message_name: %s new_message_name: %s", sMessageProxyInfo.szServiceName.c_str(), nMessageID, iterName->second.c_str(), sMessageProxyInfo.szMessageName.c_str());
			return;
		}

		SMessageProxyGroupInfo& sMessageProxyGroupInfo = this->m_mapMessageProxyGroupInfo[sMessageProxyInfo.szMessageName];
		if (sMessageProxyGroupInfo.mapMessageProxyInfo.empty())
			sMessageProxyGroupInfo.nTotalWeight = 0;

		auto iter = sMessageProxyGroupInfo.mapMessageProxyInfo.find(sMessageProxyInfo.szServiceName);
		if (iter != sMessageProxyGroupInfo.mapMessageProxyInfo.end())
		{
			PrintWarning("message proxy info is exist service_name: %s message_name: %s", sMessageProxyInfo.szServiceName.c_str(), sMessageProxyInfo.szMessageName.c_str());
			return;
		}

		sMessageProxyGroupInfo.mapMessageProxyInfo[sMessageProxyInfo.szServiceName] = sMessageProxyInfo;
		sMessageProxyGroupInfo.nTotalWeight += sMessageProxyInfo.nWeight;
		if (!sMessageProxyInfo.szServiceGroup.empty())
			sMessageProxyGroupInfo.mapGroupWeight[sMessageProxyInfo.szServiceGroup] += sMessageProxyInfo.nWeight;
		
		this->m_mapMessageName[nMessageID] = sMessageProxyInfo.szMessageName;

		PrintInfo("add message proxy info service_name: %s service_group: %s message_name: %s", sMessageProxyInfo.szServiceName.c_str(), sMessageProxyInfo.szServiceGroup.c_str(), sMessageProxyInfo.szMessageName.c_str());
	}

	const SMessageProxyGroupInfo* CCoreServiceProxy::getMessageProxyGroupInfo(const std::string& szMessageName) const
	{
		auto iter = this->m_mapMessageProxyGroupInfo.find(szMessageName);
		if (iter == this->m_mapMessageProxyGroupInfo.end())
			return nullptr;

		return &iter->second;
	}

	const SMetaMessageProxyInfo* CCoreServiceProxy::getMetaMessageProxyInfo(const std::string& szMessageName) const
	{
		uint32_t nMessageID = base::hash(szMessageName.c_str());

		return this->getMetaMessageProxyInfo(nMessageID);
	}

	const SMetaMessageProxyInfo* CCoreServiceProxy::getMetaMessageProxyInfo(uint32_t nMessageID) const
	{
		auto iter = this->m_mapMetaMessageProxyInfo.find(nMessageID);
		if (iter == this->m_mapMetaMessageProxyInfo.end())
		{
			if (!this->m_bProxyAny)
				return nullptr;
			else
				return &this->m_sAnyMetaMessageProxyInfo;
		}

		return &iter->second;
	}

	uint32_t CCoreServiceProxy::getProxyInvokTimeout(uint32_t nMessageID) const
	{
		const SMetaMessageProxyInfo* pMetaMessageProxyInfo = this->getMetaMessageProxyInfo(nMessageID);
		if (nullptr == pMetaMessageProxyInfo)
			return this->m_nProxyInvokTimeout;

		return pMetaMessageProxyInfo->nTimeout;
	}

	void CCoreServiceProxy::addService(const SServiceBaseInfo& sServiceBaseInfo)
	{
		DebugAst(!sServiceBaseInfo.szName.empty());
		DebugAst(this->m_mapServiceBaseInfo.find(sServiceBaseInfo.szName) == this->m_mapServiceBaseInfo.end());

		this->m_mapServiceBaseInfo[sServiceBaseInfo.szName] = sServiceBaseInfo;

		PrintInfo("add proxy service service_name: %s", sServiceBaseInfo.szName.c_str());
	}

	void CCoreServiceProxy::delService(const std::string& szServiceName)
	{
		auto iter = this->m_mapServiceBaseInfo.find(szServiceName);
		if (iter == this->m_mapServiceBaseInfo.end())
			return;

		for (auto iter = this->m_mapMessageProxyGroupInfo.begin(); iter != this->m_mapMessageProxyGroupInfo.end(); ++iter)
		{
			SMessageProxyGroupInfo& sMessageProxyGroupInfo = iter->second;
			auto iterService = sMessageProxyGroupInfo.mapMessageProxyInfo.find(szServiceName);
			if (!iterService->second.szServiceGroup.empty())
			{
				int32_t& nTotalWeight = sMessageProxyGroupInfo.mapGroupWeight[iterService->second.szServiceGroup];
				nTotalWeight -= iterService->second.nWeight;
				if (nTotalWeight < 0)
					nTotalWeight = 0;
			}

			sMessageProxyGroupInfo.nTotalWeight -= iterService->second.nWeight;
			if (sMessageProxyGroupInfo.nTotalWeight < 0)
				sMessageProxyGroupInfo.nTotalWeight = 0;

			sMessageProxyGroupInfo.mapMessageProxyInfo.erase(iterService);
		}

		this->m_mapServiceBaseInfo.erase(iter);

		CCoreServiceKitImpl::Inst()->getTransporter()->delCacheMessage(szServiceName);

		PrintInfo("del other service service_name: %s", szServiceName.c_str());
	}

	const SServiceBaseInfo* CCoreServiceProxy::getServiceBaseInfo(const std::string& szServiceName) const
	{
		auto iter = this->m_mapServiceBaseInfo.find(szServiceName);
		if (iter == this->m_mapServiceBaseInfo.end())
			return nullptr;
		
		return &iter->second;
	}

	bool CCoreServiceProxy::addConnectionToService(CCoreConnectionToService* pCoreConnectionToService)
	{
		DebugAstEx(pCoreConnectionToService != nullptr, false);

		if (this->m_mapCoreConnectionToService.find(pCoreConnectionToService->getServiceName()) != this->m_mapCoreConnectionToService.end())
		{
			PrintWarning("dup service service_name: %s remote_addr: %s %d", pCoreConnectionToService->getServiceName().c_str(), pCoreConnectionToService->getRemoteAddr().szHost, pCoreConnectionToService->getRemoteAddr().nPort);
			return false;
		}

		this->m_mapCoreConnectionToService[pCoreConnectionToService->getServiceName()] = pCoreConnectionToService;

		CCoreServiceKitImpl::Inst()->getTransporter()->sendCacheMessage(pCoreConnectionToService->getServiceName());

		return true;
	}

	CCoreConnectionToService* CCoreServiceProxy::getConnectionToService(const std::string& szName) const
	{
		auto iter = this->m_mapCoreConnectionToService.find(szName);
		if (iter == this->m_mapCoreConnectionToService.end())
			return nullptr;

		return iter->second;
	}

	void CCoreServiceProxy::delConnectionToService(const std::string& szName)
	{
		auto iter = this->m_mapCoreConnectionToService.find(szName);
		if (iter == this->m_mapCoreConnectionToService.end())
			return;

		this->m_mapCoreConnectionToService.erase(iter);
	}

	CCoreConnectionFromService* CCoreServiceProxy::getConnectionFromService(const std::string& szName) const
	{
		auto iter = this->m_mapCoreConnectionFromService.find(szName);
		if (iter == this->m_mapCoreConnectionFromService.end())
			return nullptr;

		return iter->second;
	}

	bool CCoreServiceProxy::addConnectionFromService(CCoreConnectionFromService* pCoreConnectionFromService)
	{
		DebugAstEx(pCoreConnectionFromService != nullptr, false);

		auto iter = this->m_mapCoreConnectionFromService.find(pCoreConnectionFromService->getServiceName());
		if (iter != this->m_mapCoreConnectionFromService.end())
		{
			PrintWarning("dup service service_name: %s remote_addr: %s %d", pCoreConnectionFromService->getServiceName().c_str(), pCoreConnectionFromService->getRemoteAddr().szHost, pCoreConnectionFromService->getRemoteAddr().nPort);
			return false;
		}

		this->m_mapCoreConnectionFromService[pCoreConnectionFromService->getServiceName()] = pCoreConnectionFromService;

		return true;
	}

	void CCoreServiceProxy::delConnectionFromService(const std::string& szName)
	{
		auto iter = this->m_mapCoreConnectionFromService.find(szName);
		if (iter == this->m_mapCoreConnectionFromService.end())
			return;

		this->m_mapCoreConnectionFromService.erase(iter);
	}

	const std::string& CCoreServiceProxy::getMessageName(uint32_t nMessageID) const
	{
		auto iter = this->m_mapMessageName.find(nMessageID);
		if (iter == this->m_mapMessageName.end())
		{
			static std::string s_szName;
			return s_szName;
		}

		return iter->second;
	}

}
