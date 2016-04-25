#include "stdafx.h"
#include "service_mgr.h"
#include "core_connection_to_service.h"
#include "core_connection_from_service.h"
#include "core_connection_to_master.h"
#include "message_dispatcher.h"
#include "base_connection_mgr.h"
#include "core_app.h"

core::CServiceMgr* g_pServiceMgr = nullptr;

namespace core
{
	CServiceMgr::CServiceMgr()
		: m_nMasterPort(0)
	{
		this->m_tickCheckConnectMaster.setCallback(std::bind(&CServiceMgr::onCheckConnectMaster, this, std::placeholders::_1));
	}

	CServiceMgr::~CServiceMgr()
	{
		
	}

	bool CServiceMgr::init(bool bNormalService, const std::string& szMasterHost, uint16_t nMasterPort)
	{
		CBaseApp::Inst()->getBaseConnectionMgr()->setConnectRefuseCallback(std::bind(&CServiceMgr::onConnectRefuse, this, std::placeholders::_1));

		const SServiceBaseInfo& sServiceBaseInfo = CBaseApp::Inst()->getServiceBaseInfo();
		if (bNormalService && sServiceBaseInfo.nPort != 0 && sServiceBaseInfo.szHost[0] != 0)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->listen(sServiceBaseInfo.szHost, sServiceBaseInfo.nPort, "", _GET_CLASS_NAME(CCoreConnectionFromService), sServiceBaseInfo.nSendBufSize, sServiceBaseInfo.nRecvBufSize, nullptr))
				return false;
		}

		this->m_szMasterHost = szMasterHost;
		this->m_nMasterPort = nMasterPort;
		
		if (!this->m_szMasterHost.empty() && this->m_nMasterPort != 0)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, "master", _GET_CLASS_NAME(CCoreConnectionToMaster), 1024, 1024, nullptr))
				return false;

			CBaseApp::Inst()->registerTicker(&this->m_tickCheckConnectMaster, 5 * 1000, 5 * 1000, 0);
		}

		return true;
	}

	void CServiceMgr::onCheckConnectMaster(uint64_t nContext)
	{
		if ( !this->m_szMasterHost.empty() && this->m_nMasterPort != 0 && this->getConnectionToMaster() == nullptr)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, "master", _GET_CLASS_NAME(CCoreConnectionToMaster), 1024, 1024, nullptr))
			{
				PrintWarning("connect master error");
			}
		}
	}

	void CServiceMgr::onConnectRefuse(const std::string& szContext)
	{
		CCoreApp::Inst()->getTransport()->onConnectRefuse(szContext);
	}

	CCoreConnectionToService* CServiceMgr::getConnectionToService(const std::string& szName) const
	{
		auto iter = this->m_mapConnectionToService.find(szName);
		if (iter == this->m_mapConnectionToService.end())
			return nullptr;

		return iter->second;
	}

	bool CServiceMgr::addConnectionToService(CCoreConnectionToService* pCoreConnectionToService)
	{
		DebugAstEx(pCoreConnectionToService != nullptr, false);

		if (this->m_mapConnectionToService.find(pCoreConnectionToService->getServiceName()) != this->m_mapConnectionToService.end())
		{
			PrintWarning("dup service service_name: %s remote_addr: %s %d", pCoreConnectionToService->getServiceName().c_str(), pCoreConnectionToService->getRemoteAddr().szHost, pCoreConnectionToService->getRemoteAddr().nPort);
			return false;
		}

		this->m_mapConnectionToService[pCoreConnectionToService->getServiceName()] = pCoreConnectionToService;

		CCoreApp::Inst()->getTransport()->sendCacheMessage(pCoreConnectionToService->getServiceName());

		return true;
	}

	void CServiceMgr::delConnectionToService(const std::string& szName)
	{
		auto iter = this->m_mapConnectionToService.find(szName);
		if (iter == this->m_mapConnectionToService.end())
			return;

		this->m_mapConnectionToService.erase(iter);
	}

	CCoreConnectionFromService* CServiceMgr::getConnectionFromService(const std::string& szName) const
	{
		auto iter = this->m_mapConnectionFromService.find(szName);
		if (iter == this->m_mapConnectionFromService.end())
			return nullptr;

		return iter->second;
	}

	bool CServiceMgr::addConnectionFromService(CCoreConnectionFromService* pCoreConnectionFromService)
	{
		DebugAstEx(pCoreConnectionFromService != nullptr, false);

		auto iter = this->m_mapConnectionFromService.find(pCoreConnectionFromService->getServiceName());
		if (iter != this->m_mapConnectionFromService.end())
		{
			PrintWarning("dup service service_name: %s remote_addr: %s %d", pCoreConnectionFromService->getServiceName().c_str(), pCoreConnectionFromService->getRemoteAddr().szHost, pCoreConnectionFromService->getRemoteAddr().nPort);
			return false;
		}

		this->m_mapConnectionFromService[pCoreConnectionFromService->getServiceName()] = pCoreConnectionFromService;
	
		return true;
	}

	void CServiceMgr::delConnectionFromService(const std::string& szName)
	{
		auto iter = this->m_mapConnectionFromService.find(szName);
		if (iter == this->m_mapConnectionFromService.end())
			return;

		this->m_mapConnectionFromService.erase(iter);
	}

	CCoreConnectionToMaster* CServiceMgr::getConnectionToMaster() const
	{
		std::vector<CBaseConnection*> vecBaseConnection;
		CCoreApp::Inst()->getCoreConnectionMgr()->getBaseConnection(_GET_CLASS_ID(CCoreConnectionToMaster), vecBaseConnection);
		if (vecBaseConnection.empty())
			return nullptr;

		return dynamic_cast<CCoreConnectionToMaster*>(vecBaseConnection[0]);
	}

	const SServiceBaseInfo* CServiceMgr::getServiceBaseInfo(const std::string& szName) const
	{
		auto iter = this->m_mapServiceBaseInfo.find(szName);
		if (iter == this->m_mapServiceBaseInfo.end())
			return nullptr;

		return &iter->second;
	}

	void CServiceMgr::getServiceName(const std::string& szType, std::vector<std::string>& vecServiceName) const
	{
		for (auto iter = this->m_mapServiceBaseInfo.begin(); iter != this->m_mapServiceBaseInfo.end(); ++iter)
		{
			if (iter->second.szType == szType)
				vecServiceName.push_back(iter->second.szName);
		}
	}

	void CServiceMgr::addOtherService(const SServiceBaseInfo& sServiceBaseInfo)
	{
		DebugAst(!sServiceBaseInfo.szName.empty());
		DebugAst(this->m_mapServiceBaseInfo.find(sServiceBaseInfo.szName) == this->m_mapServiceBaseInfo.end());

		this->m_mapServiceBaseInfo[sServiceBaseInfo.szName] = sServiceBaseInfo;

		PrintInfo("add other service service_name: %s", sServiceBaseInfo.szName.c_str());
	}

	void CServiceMgr::delOtherService(const std::string& szServiceName)
	{
		auto iter = this->m_mapServiceBaseInfo.find(szServiceName);
		if (iter == this->m_mapServiceBaseInfo.end())
			return;

		CCoreApp::Inst()->getMessageDirectory()->delOtherServiceMessageInfo(szServiceName);

		this->m_mapServiceBaseInfo.erase(iter);

		CCoreApp::Inst()->getTransport()->delCacheMessage(szServiceName);

		PrintInfo("del other service service_name: %s", szServiceName.c_str());
	}
}