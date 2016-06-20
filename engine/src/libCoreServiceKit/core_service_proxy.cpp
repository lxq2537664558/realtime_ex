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

	bool CCoreServiceProxy::addServiceConnection(CCoreServiceConnection* pCoreConnectionToService)
	{
		DebugAstEx(pCoreConnectionToService != nullptr, false);

		if (this->m_mapCoreServiceConnection.find(pCoreConnectionToService->getServiceName()) != this->m_mapCoreServiceConnection.end())
		{
			PrintWarning("dup service service_name: %s remote_addr: %s %d", pCoreConnectionToService->getServiceName().c_str(), pCoreConnectionToService->getRemoteAddr().szHost, pCoreConnectionToService->getRemoteAddr().nPort);
			return false;
		}

		this->m_mapCoreServiceConnection[pCoreConnectionToService->getServiceName()] = pCoreConnectionToService;

		CCoreServiceKitImpl::Inst()->getTransporter()->sendCacheMessage(pCoreConnectionToService->getServiceName());

		return true;
	}

	CCoreServiceConnection* CCoreServiceProxy::getServiceConnection(const std::string& szName) const
	{
		auto iter = this->m_mapCoreServiceConnection.find(szName);
		if (iter == this->m_mapCoreServiceConnection.end())
			return nullptr;

		return iter->second;
	}

	void CCoreServiceProxy::delServiceConnection(const std::string& szName)
	{
		auto iter = this->m_mapCoreServiceConnection.find(szName);
		if (iter == this->m_mapCoreServiceConnection.end())
			return;

		this->m_mapCoreServiceConnection.erase(iter);
	}

}
