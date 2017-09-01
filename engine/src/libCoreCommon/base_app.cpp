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

#include "libBaseCommon/function_util.h"
#include "libBaseCommon/exception_handler.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/logger.h"

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

	bool CBaseApp::runAndServe(const std::string& szInstanceName, const std::string& szConfig)
	{
		return CCoreApp::Inst()->runAndServe(szInstanceName, szConfig);
	}

	void CBaseApp::registerTicker(uint8_t nType, uint32_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreApp::Inst()->registerTicker(nType, nFromServiceID, nFromActorID, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CBaseApp::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	CBaseConnectionMgr*	CBaseApp::getBaseConnectionMgr() const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgr();
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
		CCoreService* pCoreService = CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->getCoreServiceByID(nServiceID);
		if (pCoreService == nullptr)
			return nullptr;

		return pCoreService->getServiceBase();
	}

	uint32_t CBaseApp::getServiceID(const std::string& szName) const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceID(szName);
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
		base::enableProfiling(bEnable);
	}

	uint32_t CBaseApp::getQPS() const
	{
		return CCoreApp::Inst()->getQPS();
	}

	void CBaseApp::debugLog(bool bEnable)
	{
		base::debugLog(bEnable);
	}

	bool CBaseApp::isLocalService(uint32_t nServiceID) const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getCoreServiceMgr()->isLocalService(nServiceID);
	}

	const std::vector<uint32_t>& CBaseApp::getServiceIDByTypeName(const std::string& szName) const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceIDByTypeName(szName);
	}
}

extern "C"
#ifdef _WIN32
__declspec(dllexport)
#endif
void runAndServe(const std::string& szInstanceName, const std::string& szConfig)
{
	core::CBaseApp* pBaseApp = new core::CBaseApp();
	pBaseApp->runAndServe(szInstanceName, szConfig);
	SAFE_DELETE(pBaseApp);
}