#include "stdafx.h"
#include "core_service_proxy.h"
#include "core_service_kit_impl.h"

namespace core
{

	CCoreServiceProxy::CCoreServiceProxy()
	{

	}

	CCoreServiceProxy::~CCoreServiceProxy()
	{

	}

	bool CCoreServiceProxy::init()
	{
		return true;
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

}
