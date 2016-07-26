#include "stdafx.h"
#include "core_service_proxy.h"
#include "core_service_kit_impl.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

#define _CHECK_CONNECT_TIME 5000

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

	void CCoreServiceProxy::addServiceBaseInfo(const SServiceBaseInfo& sServiceBaseInfo)
	{
		DebugAst(!sServiceBaseInfo.szName.empty());
		DebugAst(this->m_mapServiceInfo.find(sServiceBaseInfo.szName) == this->m_mapServiceInfo.end());

		SServiceInfo& sServiceInfo = this->m_mapServiceInfo[sServiceBaseInfo.szName];
		sServiceInfo.pCoreConnectionFromService = nullptr;
		sServiceInfo.pCoreConnectionToService = nullptr;
		sServiceInfo.sServiceBaseInfo = sServiceBaseInfo;
		sServiceInfo.pTicker = new CTicker();
		sServiceInfo.pTicker->setCallback(std::bind(&SServiceInfo::onTicker, &sServiceInfo, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(sServiceInfo.pTicker, 0, _CHECK_CONNECT_TIME, 0);

		PrintInfo("add proxy service service_name: %s", sServiceBaseInfo.szName.c_str());
	}
	
	void CCoreServiceProxy::delServiceBaseInfo(const std::string& szServiceName)
	{
		auto iter = this->m_mapServiceInfo.find(szServiceName);
		if (iter == this->m_mapServiceInfo.end())
			return;

		SAFE_DELETE(iter->second.pTicker);

		this->m_mapServiceInfo.erase(iter);

		PrintInfo("del other service service_name: %s", szServiceName.c_str());
	}

	const SServiceBaseInfo* CCoreServiceProxy::getServiceBaseInfo(const std::string& szServiceName) const
	{
		auto iter = this->m_mapServiceInfo.find(szServiceName);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;
		
		return &iter->second.sServiceBaseInfo;
	}

	bool CCoreServiceProxy::addCoreConnectionToService(CCoreConnectionToService* pCoreConnectionToService)
	{
		DebugAstEx(pCoreConnectionToService != nullptr, false);

		auto iter = this->m_mapServiceInfo.find(pCoreConnectionToService->getServiceName());
		if (iter == this->m_mapServiceInfo.end())
		{
			PrintWarning("unknwon service service_name: %s remote_addr: %s %d", pCoreConnectionToService->getServiceName().c_str(), pCoreConnectionToService->getRemoteAddr().szHost, pCoreConnectionToService->getRemoteAddr().nPort);
			return false;
		}

		DebugAstEx(iter->second.pCoreConnectionToService == nullptr, false);

		iter->second.pCoreConnectionToService = pCoreConnectionToService;

		return true;
	}

	CCoreConnectionToService* CCoreServiceProxy::getCoreConnectionToService(const std::string& szServiceName) const
	{
		auto iter = this->m_mapServiceInfo.find(szServiceName);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;

		return iter->second.pCoreConnectionToService;
	}

	void CCoreServiceProxy::delCoreConnectionToService(const std::string& szServiceName)
	{
		auto iter = this->m_mapServiceInfo.find(szServiceName);
		if (iter == this->m_mapServiceInfo.end())
			return;

		iter->second.pCoreConnectionToService = nullptr;
	}

	bool CCoreServiceProxy::addCoreConnectionFromService(const std::string& szServiceName, CCoreConnectionFromService* pCoreConnectionFromService)
	{
		DebugAstEx(pCoreConnectionFromService != nullptr, false);

		auto iter = this->m_mapServiceInfo.find(szServiceName);
		if (iter == this->m_mapServiceInfo.end())
		{
			PrintWarning("unknwon service service_name: %s remote_addr: %s %d", szServiceName.c_str(), pCoreConnectionFromService->getRemoteAddr().szHost, pCoreConnectionFromService->getRemoteAddr().nPort);
			return false;
		}

		DebugAstEx(iter->second.pCoreConnectionFromService == nullptr, false);

		iter->second.pCoreConnectionFromService = pCoreConnectionFromService;

		return true;
	}

	CCoreConnectionFromService* CCoreServiceProxy::getCoreConnectionFromService(const std::string& szServiceName) const
	{
		auto iter = this->m_mapServiceInfo.find(szServiceName);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;

		return iter->second.pCoreConnectionFromService;
	}

	void CCoreServiceProxy::delCoreConnectionFromService(const std::string& szServiceName)
	{
		auto iter = this->m_mapServiceInfo.find(szServiceName);
		if (iter == this->m_mapServiceInfo.end())
			return;

		iter->second.pCoreConnectionFromService = nullptr;
	}

	void CCoreServiceProxy::SServiceInfo::onTicker(uint64_t nContext)
	{
		if (this->pCoreConnectionToService != nullptr)
			return;

		if (this->sServiceBaseInfo.nPort == 0 || this->sServiceBaseInfo.szHost.empty())
			return;

		CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->sServiceBaseInfo.szHost, this->sServiceBaseInfo.nPort, eBCT_ConnectionToService, this->sServiceBaseInfo.szName, this->sServiceBaseInfo.nSendBufSize, this->sServiceBaseInfo.nRecvBufSize, nullptr);
	}

}
