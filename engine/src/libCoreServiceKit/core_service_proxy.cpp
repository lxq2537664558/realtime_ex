#include "stdafx.h"
#include "core_service_proxy.h"
#include "core_service_kit_impl.h"

namespace core
{
#define _DEFAULT_INVOKE_TIMEOUT 10000

	CCoreServiceProxy::CCoreServiceProxy()
		: m_nProxyInvokTimeout(_DEFAULT_INVOKE_TIMEOUT)
		, m_nProxyRetries(0)
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

			uint32_t nMessageID = base::hash(sMetaMessageProxyInfo.szMessageName.c_str());
			auto iter = this->m_mapMetaMessageProxyInfo.find(nMessageID);
			if (iter != this->m_mapMetaMessageProxyInfo.end())
			{
				PrintWarning("dup message name message_id: %d message_name: [%s, %s]", nMessageID, iter->second.szMessageName.c_str(), sMetaMessageProxyInfo.szMessageName.c_str());
				return false;
			}

			this->m_mapMetaMessageProxyInfo[nMessageID] = sMetaMessageProxyInfo;
		}

		return true;
	}

	void CCoreServiceProxy::addMessageProxyInfo(const SMessageProxyInfo& sMessageProxyInfo)
	{
		uint32_t nMessageID = base::hash(sMessageProxyInfo.szMessageName.c_str());
		if (this->m_mapMetaMessageProxyInfo.find(nMessageID) == this->m_mapMetaMessageProxyInfo.end())
			return;

		SMessageProxyGroupInfo& sMessageProxyGroupInfo = this->m_mapMessageProxyGroupInfo[sMessageProxyInfo.szMessageName];
		
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
			return nullptr;

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

}
