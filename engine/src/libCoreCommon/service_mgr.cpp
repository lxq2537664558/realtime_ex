#include "stdafx.h"
#include "service_mgr.h"
#include "connection_to_service.h"
#include "connection_from_service.h"
#include "connection_to_master.h"
#include "message_dispatcher.h"
#include "base_connection_mgr.h"
#include "core_app.h"

core::CServiceMgr* g_pServiceMgr = nullptr;

namespace core
{
	CServiceMgr::CServiceMgr()
		: m_nMasterPort(0)
		, m_bMasterRefuse(false)
	{
		this->m_tickCheckConnect.setCallback(std::bind(&CServiceMgr::onCheckConnect, this, std::placeholders::_1));
	}

	CServiceMgr::~CServiceMgr()
	{
		
	}

	bool CServiceMgr::init(bool bNormalService, const std::string& szMasterHost, uint16_t nMasterPort)
	{
		CBaseApp::Inst()->getBaseConnectionMgr()->setConnectRefuseCallback(std::bind(&CServiceMgr::onConnectRefuse, this, std::placeholders::_1));
		CBaseApp::Inst()->registerTicker(&this->m_tickCheckConnect, 5 * 1000, 5 * 1000, 0);

		const SServiceBaseInfo& sServiceBaseInfo = CBaseApp::Inst()->getServiceBaseInfo();
		if (bNormalService && sServiceBaseInfo.nPort != 0 && sServiceBaseInfo.szHost[0] != 0)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->listen(sServiceBaseInfo.szHost, sServiceBaseInfo.nPort, "", _GET_CLASS_NAME(CConnectionFromService), sServiceBaseInfo.nSendBufSize, sServiceBaseInfo.nRecvBufSize, nullptr))
				return false;
		}

		this->m_szMasterHost = szMasterHost;
		this->m_nMasterPort = nMasterPort;
		this->m_bMasterRefuse = false;
		if (!this->m_szMasterHost.empty() && this->m_nMasterPort != 0)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, "master", _GET_CLASS_NAME(CConnectionToMaster), 1024, 1024, nullptr))
				return false;
		}

		return true;
	}

	void CServiceMgr::onCheckConnect(uint64_t nContext)
	{
		if (this->m_bMasterRefuse)
		{
			if (!CBaseApp::Inst()->getBaseConnectionMgr()->connect(this->m_szMasterHost, this->m_nMasterPort, "master", _GET_CLASS_NAME(CConnectionToMaster), 1024, 1024, nullptr))
			{

			}

			this->m_bMasterRefuse = false;
		}
	}

	void CServiceMgr::onConnectRefuse(const std::string& szContext)
	{
		if (szContext == "master")
		{
			this->m_bMasterRefuse = true;
			return;
		}

		CCoreApp::Inst()->getTransport()->onConnectRefuse(szContext);
	}

	CConnectionToService* CServiceMgr::getConnectionToService(const std::string& szName) const
	{
		auto iter = this->m_mapConnectionToService.find(szName);
		if (iter == this->m_mapConnectionToService.end())
			return nullptr;

		return iter->second;
	}

	void CServiceMgr::addConnectionToService(CConnectionToService* pConnectionToService)
	{
		DebugAst(pConnectionToService != nullptr);

		if (this->m_mapConnectionToService.find(pConnectionToService->getServiceName()) != this->m_mapConnectionToService.end())
		{
			PrintWarning("dup service service_name: %s remote_addr: %s %d", pConnectionToService->getServiceName().c_str(), pConnectionToService->getRemoteAddr().szHost, pConnectionToService->getRemoteAddr().nPort);
			return;
		}

		this->m_mapConnectionToService[pConnectionToService->getServiceName()] = pConnectionToService;

		CCoreApp::Inst()->getTransport()->sendCacheMessage(pConnectionToService->getServiceName());
	}

	void CServiceMgr::delConnectionToService(const std::string& szName)
	{
		auto iter = this->m_mapConnectionToService.find(szName);
		if (iter == this->m_mapConnectionToService.end())
			return;

		this->m_mapConnectionToService.erase(iter);
	}

	CConnectionFromService* CServiceMgr::getConnectionFromService(const std::string& szName) const
	{
		auto iter = this->m_mapConnectionFromService.find(szName);
		if (iter == this->m_mapConnectionFromService.end())
			return nullptr;

		return iter->second;
	}

	void CServiceMgr::addConnectionFromService(CConnectionFromService* pConnectionFromService)
	{
		DebugAst(pConnectionFromService != nullptr);

		auto iter = this->m_mapConnectionFromService.find(pConnectionFromService->getServiceName());
		DebugAst(iter == this->m_mapConnectionFromService.end());

		this->m_mapConnectionFromService[pConnectionFromService->getServiceName()] = pConnectionFromService;
	}

	void CServiceMgr::delConnectionFromService(const std::string& szName)
	{
		auto iter = this->m_mapConnectionFromService.find(szName);
		if (iter == this->m_mapConnectionFromService.end())
			return;

		this->m_mapConnectionFromService.erase(iter);
	}

	CConnectionToMaster* CServiceMgr::getConnectionToMaster() const
	{
		std::vector<CBaseConnection*> vecBaseConnection;
		CCoreApp::Inst()->getCoreConnectionMgr()->getBaseConnection(_GET_CLASS_ID(CConnectionToMaster), vecBaseConnection);
		if (vecBaseConnection.empty())
			return nullptr;

		return dynamic_cast<CConnectionToMaster*>(vecBaseConnection[0]);
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
		for (auto iter : this->m_mapServiceBaseInfo)
		{
			if (iter.second.szType == szType)
				vecServiceName.push_back(iter.second.szName);
		}
	}

	void CServiceMgr::addOtherService(const SServiceBaseInfo& sServiceBaseInfo)
	{
		DebugAst(!sServiceBaseInfo.szName.empty());
		DebugAst(this->m_mapServiceBaseInfo.find(sServiceBaseInfo.szName) == this->m_mapServiceBaseInfo.end());

		PrintInfo("add service service_name: %s", sServiceBaseInfo.szName.c_str());

		this->m_mapServiceBaseInfo[sServiceBaseInfo.szName] = sServiceBaseInfo;
	}

	void CServiceMgr::delOtherService(const std::string& szServiceName)
	{
		auto iter = this->m_mapServiceBaseInfo.find(szServiceName);
		if (iter == this->m_mapServiceBaseInfo.end())
			return;

		CCoreApp::Inst()->getMessageDirectory()->delOtherServiceMessageInfo(szServiceName);

		this->m_mapServiceBaseInfo.erase(iter);
		PrintInfo("del service service_name: %s", szServiceName.c_str());

		CCoreApp::Inst()->getTransport()->delCacheMessage(szServiceName);
	}
}