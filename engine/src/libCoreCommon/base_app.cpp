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

	bool CBaseApp::run(int32_t argc, char** argv, const char* szConfig)
	{
		return CCoreApp::Inst()->run(argc, argv, szConfig);
	}

	void CBaseApp::registerTicker(uint32_t nType, uint64_t nFrom, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CCoreApp::Inst()->registerTicker(nType, nFrom, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CBaseApp::unregisterTicker(CTicker* pTicker)
	{
		CCoreApp::Inst()->unregisterTicker(pTicker);
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

	void CBaseApp::profiling(bool bEnable)
	{
		base::enableProfiling(bEnable);
	}

	uint32_t CBaseApp::getQPS() const
	{
		return CCoreApp::Inst()->getQPS();
	}

	void CBaseApp::busy()
	{
		CCoreApp::Inst()->busy();
	}

	void CBaseApp::debugLog(bool bEnable)
	{
		base::debugLog(bEnable);
	}
}