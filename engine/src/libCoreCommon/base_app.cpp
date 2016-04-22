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

#include "libBaseCommon/base_function.h"
#include "libBaseCommon/exception_handler.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/logger.h"

#include "base_app.h"
#include "core_app.h"
#include "core_connection_mgr.h"


// 放这里为了调试或者看dump的时候方便
core::CBaseApp* g_pCoreApp = nullptr;

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
		return g_pCoreApp;
	}

	bool CBaseApp::run(bool bNormalService, int32_t argc, char** argv, const char* szConfig)
	{
		return CCoreApp::Inst()->run(bNormalService, argc, argv, szConfig);
	}

	const SServiceBaseInfo& CBaseApp::getServiceBaseInfo() const
	{
		return CCoreApp::Inst()->getServiceBaseInfo();
	}

	void CBaseApp::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreApp::Inst()->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CBaseApp::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
	}

	int64_t CBaseApp::getLogicTime() const
	{
		return CCoreApp::Inst()->getLogicTime();
	}

	CBaseConnectionMgr* CBaseApp::getBaseConnectionMgr() const
	{
		return CCoreApp::Inst()->getCoreConnectionMgr()->getBaseConnectionMgr();
	}

	const std::string& CBaseApp::getConfigFileName() const
	{
		return CCoreApp::Inst()->getConfigFileName();
	}

	base::CWriteBuf& CBaseApp::getWriteBuf() const
	{
		return CCoreApp::Inst()->getWriteBuf();
	}

	void CBaseApp::doQuit()
	{
		PrintInfo("CBaseApp::doQuit");
		
		CCoreApp::Inst()->doQuit();
	}

	void CBaseApp::registLoadBalancePolicy(ILoadBalancePolicy* pLoadBalancePolicy)
	{
		CCoreApp::Inst()->getLoadBalancePolicyMgr()->registLoadBalancePolicy(pLoadBalancePolicy);
	}

	ILoadBalancePolicy* CBaseApp::getLoadBalancePolicy(uint32_t nID) const
	{
		return CCoreApp::Inst()->getLoadBalancePolicyMgr()->getLoadBalancePolicy(nID);
	}

	const std::set<std::string>& CBaseApp::getServiceName(const std::string& szMessageName) const
	{
		return CCoreApp::Inst()->getMessageDirectory()->getOtherServiceName(szMessageName);
	}

	const std::string& CBaseApp::getMessageName(uint32_t nMessageID) const
	{
		return CCoreApp::Inst()->getMessageDirectory()->getMessageName(nMessageID);
	}
}