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

#include "core_app.h"
#include "base_app.h"
#include "base_object.h"
#include "ticker_mgr.h"
#include "monitor.h"
#include "core_connection_mgr.h"
#include "message_dispatcher.h"
#include "message_registry.h"
#include "cluster_invoker.h"
#include "service_mgr.h"

#include "tinyxml2/tinyxml2.h"

#ifndef _WIN32
///< 环境变量
extern char **environ;
// 修改进程信息，方便运维管理
static bool initProcessInfo(size_t argc, char** argv, const char* title)
{
	// 把整个环境变量拷贝出去
	size_t environ_size = 0;
	for (size_t i = 0; environ[i] != nullptr; ++i)
	{
		environ_size += strlen(environ[i]) + 1;
	}
	char* new_environ = new char[environ_size];
	for (size_t i = 0; environ[i] != nullptr; ++i)
	{
		memcpy(new_environ, environ[i], strlen(environ[i]) + 1);
		environ[i] = new_environ;
		new_environ += strlen(environ[i]) + 1;
	}

	size_t argv_size = 0;
	for (size_t i = 0; argv[i] != nullptr; ++i)
	{
		argv_size += strlen(argv[i]) + 1;
	}

	char** new_argv = new char*[argc];
	char* new_argv_buff = new char[argv_size + strlen(title) + 1];
	for (size_t i = 0; i < argc; ++i)
	{
		uint32_t size = 0;
		if (i == 0)
		{
			size = strlen(title) + 1;
			memcpy(new_argv_buff, title, size);
		}
		else
		{
			size = strlen(argv[i]) + 1;
			memcpy(new_argv_buff, argv[i], size);
		}
		new_argv[i] = new_argv_buff;
		new_argv_buff += size;
	}
	// 这里越界也没关系
	memcpy(argv[0], new_argv[0], argv_size + strlen(title) + 1);
	for (size_t i = 0; i < argc; ++i)
		argv[i] = new_argv[i];

	return true;
}
#endif

namespace core
{
	CCoreApp::CCoreApp()
		: m_pTickerMgr(nullptr)
		, m_pCoreConnectionMgr(nullptr)
		, m_pServiceMgr(nullptr)
		, m_pTransport(nullptr)
		, m_pMessageDirectory(nullptr)
		, m_pLoadBalancePolicyMgr(nullptr)
		, m_nRunState(eARS_Start)
		, m_bMarkQuit(false)
		, m_nTotalTime(0)
		, m_nCycleCount(0)
		, m_bNormalService(true)
	{
		memset(&this->m_sServiceBaseInfo, 0, sizeof(this->m_sServiceBaseInfo));
	}

	CCoreApp::~CCoreApp()
	{

	}

	bool CCoreApp::run(bool bNormalService, int32_t argc, char** argv, const char* szConfig)
	{
		if (nullptr == szConfig)
		{
			fprintf(stderr, "nullptr == szConfig\n");
			return false;
		}

		this->m_szConfig = szConfig;

		base::setInstanceName(argv[0]);

		base::initProcessExceptionHander();
		base::initThreadExceptionHander();

		if (!this->onInit())
			return false;

		while (true)
		{
			if (!this->onProcess())
				break;
		}

		this->onDestroy();

		base::uninitProcessExceptionHander();

		return true;
	}

	const SServiceBaseInfo& CCoreApp::getServiceBaseInfo() const
	{
		return this->m_sServiceBaseInfo;
	}

	void CCoreApp::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		this->m_pTickerMgr->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CCoreApp::unregisterTicker(CTicker* pTicker)
	{
		this->m_pTickerMgr->unregisterTicker(pTicker);
	}

	int64_t CCoreApp::getLogicTime() const
	{
		return this->m_pTickerMgr->getLogicTime();
	}

	CCoreConnectionMgr* CCoreApp::getCoreConnectionMgr() const
	{
		return this->m_pCoreConnectionMgr;
	}

	CServiceMgr* CCoreApp::getServiceMgr() const
	{
		return this->m_pServiceMgr;
	}

	CTransport* CCoreApp::getTransport() const
	{
		return this->m_pTransport;
	}

	CMessageDirectory* CCoreApp::getMessageDirectory() const
	{
		return this->m_pMessageDirectory;
	}

	const std::string& CCoreApp::getConfigFileName() const
	{
		return this->m_szConfig;
	}

	base::CWriteBuf& CCoreApp::getWriteBuf() const
	{
		const_cast<base::CWriteBuf&>(this->m_writeBuf).clear();
		return const_cast<base::CWriteBuf&>(this->m_writeBuf);
	}

	bool CCoreApp::onInit()
	{
		// 首先获取可执行文件目录
		char szBinPath[MAX_PATH] = { 0 };
		_getcwd(szBinPath, MAX_PATH);

		// 切换到根目录
#ifdef _WIN32
		_chdir("../../../");
#else
		_chdir("../../");
#endif
		char szCurPath[MAX_PATH] = { 0 };
		_getcwd(szCurPath, MAX_PATH);
		base::setCurrentWorkPath(szCurPath);

		std::string szConfig = szCurPath;
		szConfig += "/etc/";
		szConfig += this->m_szConfig;

		this->m_szConfig = szConfig;

		tinyxml2::XMLDocument* pConfigXML = new tinyxml2::XMLDocument();
		if (pConfigXML->LoadFile(this->m_szConfig.c_str()) != tinyxml2::XML_SUCCESS)
		{
			fprintf(stderr, "load etc config error\n");
			return false;
		}
		tinyxml2::XMLElement* pRootXML = pConfigXML->RootElement();
		if (pRootXML == nullptr)
		{
			fprintf(stderr, "pRootXML == nullptr\n");
			return false;
		}

		tinyxml2::XMLElement* pServiceInfoXML = pRootXML->FirstChildElement("service_info");
		if (pServiceInfoXML == nullptr)
		{
			fprintf(stderr, "pServerInfoXML == nullptr\n");
			return false;
		}

		// 切换到工作目录
		const char* szWorkPath = pServiceInfoXML->Attribute("work_path");
		if (szWorkPath == nullptr)
		{
			fprintf(stderr, "szWorkPath == nullptr\n");
			return false;
		}

		int32_t nRes = _chdir(szWorkPath);
		if (nRes != 0)
		{
			fprintf(stderr, "switch work dir error\n");
			return false;
		}

		// 设置服务器时间
		tinyxml2::XMLElement* pServerTimeXML = pServiceInfoXML->FirstChildElement("time");
		if (pServerTimeXML != nullptr && pServerTimeXML->IntAttribute("enable") != 0)
		{
			int32_t nYear = pServerTimeXML->IntAttribute("year");
			int32_t nMonth = pServerTimeXML->IntAttribute("month");
			int32_t nDay = pServerTimeXML->IntAttribute("day");
			int32_t nHour = pServerTimeXML->IntAttribute("hour");
			int32_t nMinute = pServerTimeXML->IntAttribute("minute");
			int32_t nSecond = pServerTimeXML->IntAttribute("second");

			base::STime time;
			time.nYear = nYear;
			time.nMon = nMonth;
			time.nDay = nDay;
			time.nHour = nHour;
			time.nMin = nMinute;
			time.nSec = nSecond;

			int64_t nTime = base::getLocalTimeByTM(time);
			nTime = base::local2GmtTime(nTime);
			base::setGmtTime(nTime);
		}

#ifndef _WIN32
		// 避免在向一个已经关闭的socket写数据导致进程终止
		struct sigaction sig_action;
		memset(&sig_action, 0, sizeof(sig_action));
		sig_action.sa_handler = SIG_IGN;
		sig_action.sa_flags = 0;
		sigaction(SIGPIPE, &sig_action, 0);

		// 避免终端断开发送SIGHUP信号导致进程终止
		memset(&sig_action, 0, sizeof(sig_action));
		sig_action.sa_handler = SIG_IGN;
		sig_action.sa_flags = 0;
		sigaction(SIGHUP, &sig_action, 0);

		// 退出
		memset(&sig_action, 0, sizeof(sig_action));
		sig_action.sa_sigaction = [](int32_t signo, siginfo_t*, void* ptr)->void
		{
			if (CCoreApp::Inst()->m_nRunState != eARS_Normal)
				return;

			PrintInfo("server start quit");
			CCoreApp::Inst()->m_nRunState = eARS_Quitting;
		};
		sig_action.sa_flags = SA_SIGINFO;
		sigaction(SIGUSR1, &sig_action, 0);

		// 写PID文件
		char szPID[MAX_PATH] = { 0 };
		base::crt::snprintf(szPID, _countof(szPID), "%s/%s.pid", szBinPath, base::getInstanceName());
		FILE* pFile = fopen(szPID, "w");
		if (nullptr != pFile)
		{
			char szPID[10] = { 0 };
			base::crt::snprintf(szPID, _countof(szPID), "%d", base::getCurrentProcessID());

			fwrite(szPID, 1, base::crt::strnlen(szPID, _countof(szPID)), pFile);

			fclose(pFile);
		}

		this->m_szPID = szPID;
#endif

		if (!base::initLog(true))
		{
			fprintf(stderr, "init log error\n");
			return false;
		}
		if (!base::initProfiling())
		{
			PrintWarning("base::initProfiling()");
			return false;
		}

		if (!this->m_writeBuf.init(UINT16_MAX))
		{
			PrintWarning("this->m_writeBuf.init(UINT16_MAX)");
			return false;
		}

		if (!initMonitor())
		{
			PrintWarning("initMonitor()");
			return false;
		}

		this->m_pTickerMgr = new core::CTickerMgr();

		uint32_t nMaxSocketCount = (uint32_t)pServiceInfoXML->IntAttribute("max_socket_count");
		this->m_pCoreConnectionMgr = new CCoreConnectionMgr();
		if (!this->m_pCoreConnectionMgr->init(nMaxSocketCount))
		{
			PrintWarning("this->m_pCoreConnectionMgr->init(nMaxSocketCount)");
			return false;
		}

		this->m_pTransport = new CTransport();
		if (!this->m_pTransport->init())
		{
			PrintWarning("this->m_pMessageSend->init()");
			return false;
		}

		this->m_pMessageDirectory = new CMessageDirectory();
		if (!this->m_pMessageDirectory->init())
		{
			PrintWarning("this->m_pMessageDirectory->init()");
			return false;
		}

		this->m_pLoadBalancePolicyMgr = new CLoadBalancePolicyMgr();
		if (!this->m_pLoadBalancePolicyMgr->init())
		{
			PrintWarning("this->m_pLoadBalancePolicyMgr->init()");
			return false;
		}

		if (!CClusterInvoker::Inst()->init())
		{
			PrintWarning("CClusterInvoker::Inst()->init()");
			return false;
		}

		if (!CMessageRegistry::Inst()->init())
		{
			PrintWarning("CMessageRegistry::Inst()->init()");
			return false;
		}

		if (!CMessageDispatcher::Inst()->init())
		{
			PrintWarning("CMessageDispatcher::Inst()->init()");
			return false;
		}

		const char* szServiceType = pServiceInfoXML->Attribute("service_type");
		const char* szServiceName = pServiceInfoXML->Attribute("service_name");
		const char* szServiceHost = pServiceInfoXML->Attribute("host");
		this->m_sServiceBaseInfo.szType = szServiceType;
		this->m_sServiceBaseInfo.szName = szServiceName;
		this->m_sServiceBaseInfo.szHost = szServiceHost;
		this->m_sServiceBaseInfo.nPort = (uint16_t)pServiceInfoXML->IntAttribute("port");
		this->m_sServiceBaseInfo.nRecvBufSize = pServiceInfoXML->IntAttribute("recv_buf_size");
		this->m_sServiceBaseInfo.nSendBufSize = pServiceInfoXML->IntAttribute("send_buf_size");

		std::string szMasterHost;
		uint16_t nMasterPort = 0;
		tinyxml2::XMLElement* pMasterAddrXML = pRootXML->FirstChildElement("connect_master_addr");
		if (pMasterAddrXML == nullptr)
		{
			szMasterHost = pMasterAddrXML->Attribute("host");
			nMasterPort = (uint16_t)pMasterAddrXML->IntAttribute("port");
		}
		this->m_pServiceMgr = new CServiceMgr();
		if (!this->m_pServiceMgr->init(this->m_bNormalService, szMasterHost, nMasterPort))
		{
			PrintWarning("CServiceMgr::Inst()->init(this->m_bNormalService, szMasterHost, nMasterPort)");
		}

		SAFE_DELETE(pConfigXML);

		this->m_nRunState = eARS_Normal;

		PrintInfo("CCoreApp::onInit service name: %s", this->m_sServiceBaseInfo.szName);

		return CBaseApp::Inst()->onInit();
	}

	void CCoreApp::onDestroy()
	{
		CBaseApp::Inst()->onDestroy();

		PrintInfo("CCoreApp::onDestroy");

		SAFE_DELETE(this->m_pCoreConnectionMgr);
		SAFE_DELETE(this->m_pTickerMgr);
		SAFE_DELETE(this->m_pServiceMgr);

		CMessageDispatcher::Inst()->release();
		CMessageRegistry::Inst()->release();
		CClusterInvoker::Inst()->release();

		CBaseObject::unRegistClassInfo();

		uninitMonitor();
		base::uninitProfiling();
		base::uninitLog();

#ifndef _WIN32
		// 删除pid文件
		remove(this->m_szPID.c_str());
#endif
	}

	bool CCoreApp::onProcess()
	{
		int64_t nBeginTime = base::getGmtTime();

		CBaseApp::Inst()->onBeforeFrame();

		this->m_pCoreConnectionMgr->update(this->m_pTickerMgr->getNearestTime());

		this->m_pTickerMgr->update();

		CBaseApp::Inst()->onProcess();

		CBaseApp::Inst()->onAfterFrame();

		if (this->m_nRunState == eARS_Quitting && !this->m_bMarkQuit)
		{
			this->m_bMarkQuit = true;

			PrintInfo("CCoreApp::onQuit");
			
			base::flushLog();
			CBaseApp::Inst()->onQuit();
		}
		if (this->m_nRunState == eARS_Quit)
			return false;

		int64_t nEndTime = base::getLocalTime();
		this->m_nTotalTime = this->m_nTotalTime + (uint32_t)(nEndTime - nBeginTime);

		if (this->m_nCycleCount % 100 == 0)
		{
			this->onAnalyze();
#ifdef __PROFILING_OPEN
			base::profiling(this->m_nTotalTime);
#endif
			this->m_nTotalTime = 0;
		}
		return true;
	}

	void CCoreApp::onAnalyze()
	{
	}

	void CCoreApp::doQuit()
	{
		PrintInfo("CCoreApp::doQuit");
		DebugAst(this->m_nRunState == eARS_Quitting);

		this->m_nRunState = eARS_Quit;
	}

	void CCoreApp::registLoadBalancePolicy(ILoadBalancePolicy* pLoadBalancePolicy)
	{
		this->m_pLoadBalancePolicyMgr->registLoadBalancePolicy(pLoadBalancePolicy);
	}

	ILoadBalancePolicy* CCoreApp::getLoadBalancePolicy(uint32_t nID) const
	{
		return this->m_pLoadBalancePolicyMgr->getLoadBalancePolicy(nID);
	}

	const std::vector<std::string>& CCoreApp::getMessageServiceName(uint32_t nMessageID, bool bGate) const
	{
		return this->m_pMessageDirectory->getMessageServiceName(nMessageID, bGate);
	}

}