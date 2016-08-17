#include "stdafx.h"
#include "core_service_proxy.h"
#include "core_service_app_impl.h"

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
		DebugAst(this->m_mapServiceInfo.find(sServiceBaseInfo.nID) == this->m_mapServiceInfo.end());
		DebugAst(this->m_mapServiceName.find(sServiceBaseInfo.szName) == this->m_mapServiceName.end());

		SServiceInfo& sServiceInfo = this->m_mapServiceInfo[sServiceBaseInfo.nID];
		sServiceInfo.pCoreConnectionFromService = nullptr;
		sServiceInfo.pCoreConnectionToService = nullptr;
		sServiceInfo.sServiceBaseInfo = sServiceBaseInfo;
		sServiceInfo.pTicker = std::make_unique<CTicker>();
		sServiceInfo.pTicker->setCallback(std::bind(&SServiceInfo::onTicker, &sServiceInfo, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(sServiceInfo.pTicker.get(), 0, _CHECK_CONNECT_TIME, 0);

		this->m_mapServiceName[sServiceBaseInfo.szName] = sServiceBaseInfo.nID;

		PrintInfo("add proxy service service_id: %d service_name: %s", sServiceBaseInfo.nID, sServiceBaseInfo.szName.c_str());
	}
	
	void CCoreServiceProxy::delServiceBaseInfo(uint16_t nServiceID)
	{
		auto iter = this->m_mapServiceInfo.find(nServiceID);
		if (iter == this->m_mapServiceInfo.end())
			return;

		std::string szServiceName = iter->second.sServiceBaseInfo.szName;
		this->m_mapServiceName.erase(iter->second.sServiceBaseInfo.szName);

		this->m_mapServiceInfo.erase(iter);

		PrintInfo("del other service service_id: %d service_name: %s", nServiceID, szServiceName.c_str());
	}

	uint16_t CCoreServiceProxy::getServiceID(const std::string& szServiceName) const
	{
		auto iter = this->m_mapServiceName.find(szServiceName);
		if (iter == this->m_mapServiceName.end())
			return 0;

		return iter->second;
	}

	const SServiceBaseInfo* CCoreServiceProxy::getServiceBaseInfo(uint16_t nServiceID) const
	{
		auto iter = this->m_mapServiceInfo.find(nServiceID);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;
		
		return &iter->second.sServiceBaseInfo;
	}

	bool CCoreServiceProxy::addCoreConnectionToService(CCoreConnectionToService* pCoreConnectionToService)
	{
		DebugAstEx(pCoreConnectionToService != nullptr, false);

		auto iter = this->m_mapServiceInfo.find(pCoreConnectionToService->getServiceID());
		if (iter == this->m_mapServiceInfo.end())
		{
			PrintWarning("unknwon service service_id: %d remote_addr: %s %d", pCoreConnectionToService->getServiceID(), pCoreConnectionToService->getRemoteAddr().szHost, pCoreConnectionToService->getRemoteAddr().nPort);
			return false;
		}

		DebugAstEx(iter->second.pCoreConnectionToService == nullptr, false);

		iter->second.pCoreConnectionToService = pCoreConnectionToService;

		return true;
	}

	CCoreConnectionToService* CCoreServiceProxy::getCoreConnectionToService(uint16_t nServiceID) const
	{
		auto iter = this->m_mapServiceInfo.find(nServiceID);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;

		return iter->second.pCoreConnectionToService;
	}

	void CCoreServiceProxy::delCoreConnectionToService(uint16_t nServiceID)
	{
		auto iter = this->m_mapServiceInfo.find(nServiceID);
		if (iter == this->m_mapServiceInfo.end())
			return;

		iter->second.pCoreConnectionToService = nullptr;
	}

	bool CCoreServiceProxy::addCoreConnectionFromService(uint16_t nServiceID, CCoreConnectionFromService* pCoreConnectionFromService)
	{
		DebugAstEx(pCoreConnectionFromService != nullptr, false);

		auto iter = this->m_mapServiceInfo.find(nServiceID);
		if (iter == this->m_mapServiceInfo.end())
		{
			PrintWarning("unknwon service service_id: %d remote_addr: %s %d", nServiceID, pCoreConnectionFromService->getRemoteAddr().szHost, pCoreConnectionFromService->getRemoteAddr().nPort);
			return false;
		}

		DebugAstEx(iter->second.pCoreConnectionFromService == nullptr, false);

		iter->second.pCoreConnectionFromService = pCoreConnectionFromService;

		return true;
	}

	CCoreConnectionFromService* CCoreServiceProxy::getCoreConnectionFromService(uint16_t nServiceID) const
	{
		auto iter = this->m_mapServiceInfo.find(nServiceID);
		if (iter == this->m_mapServiceInfo.end())
			return nullptr;

		return iter->second.pCoreConnectionFromService;
	}

	void CCoreServiceProxy::delCoreConnectionFromService(uint16_t nServiceID)
	{
		auto iter = this->m_mapServiceInfo.find(nServiceID);
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

		char szBuf[64] = { 0 };
		base::crt::snprintf(szBuf, _countof(szBuf), "%d", this->sServiceBaseInfo.nID);
		CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->sServiceBaseInfo.szHost, this->sServiceBaseInfo.nPort, eBCT_ConnectionToService, szBuf, this->sServiceBaseInfo.nSendBufSize, this->sServiceBaseInfo.nRecvBufSize, nullptr);
	}

}
