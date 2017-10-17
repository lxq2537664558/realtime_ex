#include "stdafx.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <fcntl.h>
#include <sys/resource.h>
#endif

#include <signal.h>
#include <iostream>
#include <string>
#include <memory>

#include "libBaseCommon/profiling.h"

#include "base_app.h"
#include "core_app.h"

// 放这里为了调试或者看dump的时候方便

namespace 
{
	core::CBaseApp* g_pBaseApp = nullptr;
}

namespace core
{
	CBaseApp::CBaseApp()
	{
		CBaseApp::Inst() = this;
	}

	CBaseApp::~CBaseApp()
	{

	}

	CBaseApp*& CBaseApp::Inst()
	{
		return g_pBaseApp;
	}

	bool CBaseApp::runAndServe(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase)
	{
		if (!CCoreApp::Inst()->runAndServe(argc, argv, vecServiceBase))
			return false;

		CCoreApp::Inst()->release();
		return true;
	}

	void CBaseApp::registerTicker(uint32_t nServiceID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreService* pCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService(nServiceID);
		if (nullptr == pCoreService)
			return;

		CCoreApp::Inst()->registerTicker(pCoreService->getMessageQueue(), pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CBaseApp::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	const SNodeBaseInfo& CBaseApp::getNodeBaseInfo() const
	{
		return CCoreApp::Inst()->getNodeBaseInfo();
	}

	uint32_t CBaseApp::getNodeID() const
	{
		return CCoreApp::Inst()->getNodeID();
	}

	CServiceBase* CBaseApp::getServiceBase(uint32_t nServiceID) const
	{
		CCoreService* pCoreService = CCoreApp::Inst()->getCoreServiceMgr()->getCoreService(nServiceID);
		if (pCoreService == nullptr)
			return nullptr;

		return pCoreService->getServiceBase();
	}

	uint32_t CBaseApp::getServiceID(const std::string& szName) const
	{
		return CCoreApp::Inst()->getServiceRegistryProxy()->getServiceID(szName);
	}

	const std::string& CBaseApp::getConfigFileName() const
	{
		return CCoreApp::Inst()->getConfigFileName();
	}

	void CBaseApp::doQuit()
	{
		PrintInfo("CBaseApp::doQuit");

		CCoreApp::Inst()->doQuit();
	}

	void CBaseApp::profiling(bool bEnable)
	{
		base::profiling::enable(bEnable);
	}

	uint32_t CBaseApp::getQPS() const
	{
		return CCoreApp::Inst()->getQPS();
	}

	void CBaseApp::debugLog(bool bEnable)
	{
		base::log::debug(bEnable);
	}

	bool CBaseApp::isLocalService(uint32_t nServiceID) const
	{
		return CCoreApp::Inst()->getCoreServiceMgr()->isLocalService(nServiceID);
	}

	const std::set<uint32_t>& CBaseApp::getServiceIDByType(const std::string& szName) const
	{
		return CCoreApp::Inst()->getServiceRegistryProxy()->getServiceIDByType(szName);
	}

	const std::vector<uint32_t>& CBaseApp::getActiveServiceIDByType(const std::string& szName) const
	{
		return CCoreApp::Inst()->getServiceRegistryProxy()->getActiveServiceIDByType(szName);
	}

	uint32_t CBaseApp::getCoroutineStackSize() const
	{
		return CCoreApp::Inst()->getCoroutineStackSize();
	}
}