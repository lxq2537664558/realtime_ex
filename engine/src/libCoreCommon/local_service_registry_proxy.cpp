#include "stdafx.h"
#include "local_service_registry_proxy.h"
#include "core_app.h"

namespace core
{
	CLocalServiceRegistryProxy::CLocalServiceRegistryProxy()
	{
	}

	CLocalServiceRegistryProxy::~CLocalServiceRegistryProxy()
	{
	}

	uint32_t CLocalServiceRegistryProxy::getServiceID(const std::string& szName)
	{
		auto iter = this->m_mapServiceNameByID.find(szName);
		if (iter == this->m_mapServiceNameByID.end())
			return 0;

		return iter->second;
	}

	std::string CLocalServiceRegistryProxy::getServiceType(uint32_t nServiceID)
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
		{
			CCoreService* pCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService(nServiceID);
			if (pCoreService == nullptr)
				return "";

			return pCoreService->getServiceBaseInfo().szType;
		}

		return iter->second.sServiceBaseInfo.szType;
	}

	std::string CLocalServiceRegistryProxy::getServiceName(uint32_t nServiceID)
	{
		auto iter = this->m_mapServiceIDByName.find(nServiceID);
		if (iter == this->m_mapServiceIDByName.end())
			return "";

		return iter->second;
	}

	const std::set<uint32_t>& CLocalServiceRegistryProxy::getServiceIDByType(const std::string& szName)
	{
		auto iter = this->m_mapServiceIDInfoByType.find(szName);
		if (iter == this->m_mapServiceIDInfoByType.end())
		{
			static std::set<uint32_t> setEmpty;
			return setEmpty;
		}

		return iter->second.setServiceID;
	}

	const std::vector<uint32_t>& CLocalServiceRegistryProxy::getActiveServiceIDByType(const std::string& szName)
	{
		auto iter = this->m_mapServiceIDInfoByType.find(szName);
		if (iter == this->m_mapServiceIDInfoByType.end())
		{
			static std::vector<uint32_t> vecEmpty;
			return vecEmpty;
		}

		return iter->second.vecActiveServiceID;
	}

	const SServiceBaseInfo* CLocalServiceRegistryProxy::getServiceBaseInfoByServiceID(uint32_t nServiceID)
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
			return nullptr;

		return &iter->second.sServiceBaseInfo;
	}

	uint64_t CLocalServiceRegistryProxy::getOtherNodeSocketIDByServiceID(uint32_t nServiceID)
	{
		auto iter = this->m_mapServiceProxyInfo.find(nServiceID);
		if (iter == this->m_mapServiceProxyInfo.end())
			return 0;

		return iter->second.nSocketID;
	}

	bool CLocalServiceRegistryProxy::isValidService(uint32_t nServiceID) const
	{
		return this->m_mapServiceIDByName.find(nServiceID) != this->m_mapServiceIDByName.end();
	}
}