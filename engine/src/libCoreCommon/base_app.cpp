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
#include "service_base_mgr.h"

// ������Ϊ�˵��Ի��߿�dump��ʱ�򷽱�
core::CBaseApp* g_pBaseApp = nullptr;

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

	bool CBaseApp::run(const std::string& szInstanceName, const std::string& szConfig)
	{
		return CCoreApp::Inst()->run(szInstanceName, szConfig);
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
		CServiceBaseImpl* pServiceBaseImpl = CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->getServiceBaseByID(nServiceID);
		if (pServiceBaseImpl == nullptr)
			return nullptr;

		return pServiceBaseImpl->getServiceBase();
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

	void CBaseApp::addGlobalBeforeFilter(uint8_t nMessageType, const std::string& szKey, const NodeGlobalFilter& callback)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->addGlobalBeforeFilter(nMessageType, szKey, callback);
	}

	void CBaseApp::delGlobalBeforeFilter(uint8_t nMessageType, const std::string& szKey)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->delGlobalBeforeFilter(nMessageType, szKey);
	}

	void CBaseApp::addGlobalAfterFilter(uint8_t nMessageType, const std::string& szKey, const NodeGlobalFilter& callback)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->addGlobalAfterFilter(nMessageType, szKey, callback);
	}

	void CBaseApp::delGlobalAfterFilter(uint8_t nMessageType, const std::string& szKey)
	{
		CCoreApp::Inst()->getLogicRunnable()->getBaseConnectionMgrImpl()->delGlobalAfterFilter(nMessageType, szKey);
	}

	bool CBaseApp::isLocalService(uint32_t nServiceID) const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getServiceBaseMgr()->isLocalService(nServiceID);
	}

	const std::vector<uint32_t>& CBaseApp::getServiceIDByTypeName(const std::string& szName) const
	{
		return CCoreApp::Inst()->getLogicRunnable()->getServiceRegistryProxy()->getServiceIDByTypeName(szName);
	}
}