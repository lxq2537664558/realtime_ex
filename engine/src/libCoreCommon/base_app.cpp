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
#include "base_app_impl.h"
#include "core_connection_mgr.h"
#include "memory_hook.h"

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

	bool CBaseApp::run(int32_t argc, char** argv, const char* szConfig)
	{
		return CBaseAppImpl::Inst()->run(argc, argv, szConfig);
	}

	void CBaseApp::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CBaseAppImpl::Inst()->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CBaseApp::unregisterTicker(CTicker* pTicker)
	{
		CBaseAppImpl::Inst()->unregisterTicker(pTicker);
	}

	int64_t CBaseApp::getLogicTime() const
	{
		return CBaseAppImpl::Inst()->getLogicTime();
	}

	CBaseConnectionMgr* CBaseApp::getBaseConnectionMgr() const
	{
		return CBaseAppImpl::Inst()->getBaseConnectionMgr();
	}

	const std::string& CBaseApp::getConfigFileName() const
	{
		return CBaseAppImpl::Inst()->getConfigFileName();
	}

	base::CWriteBuf& CBaseApp::getWriteBuf() const
	{
		return CBaseAppImpl::Inst()->getWriteBuf();
	}

	void CBaseApp::doQuit()
	{
		PrintInfo("CBaseApp::doQuit");
		
		CBaseAppImpl::Inst()->doQuit();
	}

	void CBaseApp::enableProfiling(bool bProfiling)
	{
		base::enableProfiling(bProfiling);
	}

	void CBaseApp::beginLeakChecker(bool bDetail)
	{
		beginMemoryLeakChecker(bDetail);
	}

	void CBaseApp::endLeakChecker(const char* szName)
	{
		endMemoryLeakChecker(szName);
	}
}