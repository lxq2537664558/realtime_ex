#include "stdafx.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <fcntl.h>
#include <sys/resource.h>
#include <dlfcn.h>
#endif

#include <signal.h>
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>

#include "libBaseCommon/function_util.h"
#include "libBaseCommon/exception_handler.h"
#include "libBaseCommon/time_util.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/process_util.h"
#include "libBaseCommon/logger.h"

#include "tinyxml2/tinyxml2.h"

#include "core_app.h"
#include "base_app.h"
#include "base_object.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "message_command.h"
#include "class_info_mgr.h"
#include "ticker_runnable.h"
#include "net_runnable.h"
#include "logic_runnable.h"
#include "coroutine_mgr.h"

#define _DEFAULT_HEARTBEAT_LIMIT	3
#define _DEFAULT_HEARTBEAT_TIME		10
#define _DEFAULT_SAMPLING_TIME		1000
#define _DEFAULT_INVOKE_TIMEOUT	30*1000
#define _MAX_NOFILE_SIZE 1024*10
#define _DEFAULT_COROUTINE_STACK_SIZE 128 * 1024

#ifndef _WIN32
// 环境变量
extern char **environ;

namespace
{
	// 修改进程信息，方便运维管理
	bool initProcessInfo(size_t argc, char** argv, const char* title)
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
}
#endif

namespace core
{
	CCoreApp::CCoreApp()
		: m_writeBuf(UINT16_MAX)
		, m_nQuiting(0)
		, m_nLogicThreadCount(0)
		, m_nHeartbeatLimit(_DEFAULT_HEARTBEAT_LIMIT)
		, m_nHeartbeatTime(_DEFAULT_HEARTBEAT_TIME)
		, m_nSamplingTime(_DEFAULT_SAMPLING_TIME)
		, m_nCoroutineStackSize(_DEFAULT_COROUTINE_STACK_SIZE)
		, m_nQPS(0)
		, m_nDefaultServiceInvokeTimeout(_DEFAULT_INVOKE_TIMEOUT)
		, m_pGlobalBaseConnectionMgr(nullptr)
		, m_pLogicMessageQueueMgr(nullptr)
		, m_pGlobalLogicMessageQueue(nullptr)
		, m_pCoreServiceMgr(nullptr)
		, m_pServiceRegistryProxy(nullptr)
		, m_pNodeConnectionFactory(nullptr)
	{
		this->m_pLogicMessageQueueMgr = new CLogicMessageQueueMgr();
		this->m_pGlobalLogicMessageQueue = new CLogicMessageQueue(nullptr, this->m_pLogicMessageQueueMgr);
	}

	CCoreApp::~CCoreApp()
	{

	}

	bool CCoreApp::runAndServe(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase)
	{
		if (argc < 2)
			return false;

		this->m_szConfig = argv[1];

		base::initProcessExceptionHander();
		base::initThreadExceptionHander();

		if (!this->init(argc, argv, vecServiceBase))
			return false;

		for (size_t i = 0; i < this->m_vecLogicRunnable.size(); ++i)
		{
			this->m_vecLogicRunnable[i]->join();
		}

		this->destroy();

		base::uninitProcessExceptionHander();

		return true;
	}

	void CCoreApp::registerTicker(CMessageQueue* pMessageQueue, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		CTickerRunnable::Inst()->registerTicker(pMessageQueue, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CCoreApp::unregisterTicker(CTicker* pTicker)
	{
		CTickerRunnable::Inst()->unregisterTicker(pTicker);
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

	bool CCoreApp::init(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase)
	{
#ifndef _WIN32
		struct rlimit rlp;
		if (getrlimit(RLIMIT_CORE, &rlp) != 0)
		{
			fprintf(stderr, "getrlimit error: %u", base::getLastError());
			return false;
		}
		if (rlp.rlim_max != RLIM_INFINITY)
		{
			fprintf(stderr, "core dump size error");
			return false;
		}
		rlp.rlim_cur = rlp.rlim_max;
		if (setrlimit(RLIMIT_CORE, &rlp) != 0)
		{
			fprintf(stderr, "setrlimit error: %u", base::getLastError());
			return false;
		}

		if (getrlimit(RLIMIT_NOFILE, &rlp) != 0)
		{
			fprintf(stderr, "getrlimit error: %u", base::getLastError());
			return false;
		}
		if (rlp.rlim_cur < _MAX_NOFILE_SIZE)
		{
			fprintf(stderr, "nofile too small");
			return false;
		}
#endif
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
		base::process_util::setCurrentWorkPath(szCurPath);

		std::string szConfig = szCurPath;
		szConfig += "/etc/";
		szConfig += this->m_szConfig;

		this->m_szConfig = szConfig;

		tinyxml2::XMLDocument sConfigXML;
		if (sConfigXML.LoadFile(this->m_szConfig.c_str()) != tinyxml2::XML_SUCCESS)
		{
			fprintf(stderr, "load etc config error\n");
			return false;
		}
		tinyxml2::XMLElement* pRootXML = sConfigXML.RootElement();
		if (pRootXML == nullptr)
		{
			fprintf(stderr, "pRootXML == nullptr\n");
			return false;
		}

		tinyxml2::XMLElement* pBaseInfoXML = pRootXML->FirstChildElement("base_info");
		if (pBaseInfoXML == nullptr)
		{
			fprintf(stderr, "pBaseInfoXML == nullptr\n");
			return false;
		}

		// 切换到工作目录
		const char* szWorkPath = pBaseInfoXML->Attribute("work_path");
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
		tinyxml2::XMLElement* pServerTimeXML = pBaseInfoXML->FirstChildElement("time");
		if (pServerTimeXML != nullptr && pServerTimeXML->IntAttribute("enable") != 0)
		{
			int32_t nYear = pServerTimeXML->IntAttribute("year");
			int32_t nMonth = pServerTimeXML->IntAttribute("month");
			int32_t nDay = pServerTimeXML->IntAttribute("day");
			int32_t nHour = pServerTimeXML->IntAttribute("hour");
			int32_t nMinute = pServerTimeXML->IntAttribute("minute");
			int32_t nSecond = pServerTimeXML->IntAttribute("second");

			base::time_util::STime sTime;
			sTime.nYear = nYear;
			sTime.nMon = nMonth;
			sTime.nDay = nDay;
			sTime.nHour = nHour;
			sTime.nMin = nMinute;
			sTime.nSec = nSecond;

			int64_t nTime = base::time_util::getGmtTimeByTM(sTime);
			base::time_util::setGmtTime(nTime);
		}

		tinyxml2::XMLElement* pNodeInfoXML = pRootXML->FirstChildElement("node_info");
		if (pNodeInfoXML == nullptr)
		{
			PrintWarning("pNodeInfoXML == nullptr");
			return false;
		}

		uint32_t nID = pNodeInfoXML->UnsignedAttribute("node_id");
		if (nID > UINT16_MAX)
		{
			PrintWarning("too big node id: {}", nID);
			return false;
		}
		// 加载节点基本信息
		this->m_sNodeBaseInfo.nID = nID;
		this->m_sNodeBaseInfo.szName = pNodeInfoXML->Attribute("node_name");
		if (pNodeInfoXML->Attribute("node_group") != nullptr)
			this->m_sNodeBaseInfo.szGroup = pNodeInfoXML->Attribute("node_group");
		if (pNodeInfoXML->Attribute("host") != nullptr)
			this->m_sNodeBaseInfo.szHost = pNodeInfoXML->Attribute("host");
		this->m_sNodeBaseInfo.nPort = (uint16_t)pNodeInfoXML->UnsignedAttribute("port");
		this->m_sNodeBaseInfo.nRecvBufSize = pNodeInfoXML->UnsignedAttribute("recv_buf_size");
		this->m_sNodeBaseInfo.nSendBufSize = pNodeInfoXML->UnsignedAttribute("send_buf_size");
		this->m_nDefaultServiceInvokeTimeout = pNodeInfoXML->UnsignedAttribute("invoke_timeout");
		if (this->m_nDefaultServiceInvokeTimeout == 0)
			this->m_nDefaultServiceInvokeTimeout = _DEFAULT_INVOKE_TIMEOUT;

		base::process_util::setInstanceName(this->m_sNodeBaseInfo.szName.c_str());

#ifdef _WIN32
		::SetConsoleTitleA(base::process_util::getInstanceName());
#else
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
			CCoreApp::Inst()->doQuit();
		};
		sig_action.sa_flags = SA_SIGINFO;
		sigaction(SIGUSR1, &sig_action, 0);

		// 写PID文件
		char szPID[MAX_PATH] = { 0 };
		base::function_util::snprintf(szPID, _countof(szPID), "%s/%s.pid", szBinPath, base::process_util::getInstanceName());
		FILE* pFile = fopen(szPID, "w");
		if (nullptr != pFile)
		{
			char szPID[10] = { 0 };
			base::function_util::snprintf(szPID, _countof(szPID), "%d", base::process_util::getCurrentProcessID());

			fwrite(szPID, 1, base::function_util::strnlen(szPID, _countof(szPID)), pFile);

			fclose(pFile);
		}

		this->m_szPID = szPID;
		initProcessInfo(argc, argv, base::process_util::getInstanceName());
#endif

		tinyxml2::XMLElement* pLogXML = pBaseInfoXML->FirstChildElement("log");
		if (pLogXML == nullptr)
		{
			fprintf(stderr, "pLogXML == nullptr\n");
			return false;
		}

		if (!base::log::init(pLogXML->IntAttribute("async") != 0, false, pLogXML->Attribute("path")))
		{
			fprintf(stderr, "init log error\n");
			return false;
		}

		base::log::debug(pLogXML->IntAttribute("debug") != 0);

		bool bProfiling = false;
		tinyxml2::XMLElement* pProfilingXML = pBaseInfoXML->FirstChildElement("profiling");
		if (pProfilingXML != nullptr)
		{
			this->m_nSamplingTime = (uint32_t)pProfilingXML->IntAttribute("sampling_time");
			bProfiling = true;
		}
		
		if (!base::profiling::init(bProfiling))
		{
			PrintWarning("base::initProfiling()");
			return false;
		}

		uint32_t nMaxConnectionCount = (uint32_t)pBaseInfoXML->IntAttribute("connections");
		this->m_nLogicThreadCount = std::max<uint32_t>(1, (uint32_t)pBaseInfoXML->IntAttribute("logic_threads"));
		
		// 加载服务连接心跳信息
		tinyxml2::XMLElement* pHeartbeatXML = pBaseInfoXML->FirstChildElement("heartbeat");
		if (nullptr != pHeartbeatXML)
		{
			this->m_nHeartbeatLimit = pHeartbeatXML->UnsignedAttribute("heartbeat_limit");
			this->m_nHeartbeatTime = pHeartbeatXML->UnsignedAttribute("heartbeat_time");
		}

		if (!CCoroutineMgr::Inst()->init())
		{
			PrintWarning("CCoroutineMgr::Inst()->init()");
			return false;
		}

		this->m_tickerQPS.setCallback(std::bind(&CCoreApp::onQPS, this, std::placeholders::_1));
		this->registerTicker(this->m_pGlobalLogicMessageQueue, &this->m_tickerQPS, 1000, 1000, 0);

		this->m_pGlobalBaseConnectionMgr = new CBaseConnectionMgr(this->m_pGlobalLogicMessageQueue);
		
		this->m_pNodeConnectionFactory = new CNodeConnectionFactory();
		this->m_pGlobalBaseConnectionMgr->setBaseConnectionFactory("CBaseConnectionToMaster", this->m_pNodeConnectionFactory);
		this->m_pGlobalBaseConnectionMgr->setBaseConnectionFactory("CBaseConnectionOtherNode", this->m_pNodeConnectionFactory);

		this->m_pCoreServiceMgr = new CCoreServiceMgr();
		if (!this->m_pCoreServiceMgr->init(vecServiceBase))
		{
			PrintWarning("this->m_pCoreServiceMgr->init(vecServiceBase)");
			return false;
		}

		this->m_pServiceRegistryProxy = new CServiceRegistryProxy();
		if (!this->m_pServiceRegistryProxy->init(pRootXML))
		{
			PrintWarning("this->m_pServiceRegistryProxy->init(pRootXML)");
			return false;
		}

		if (!CNetRunnable::Inst()->init(nMaxConnectionCount))
		{
			PrintWarning("CNetRunnable::Inst()->init(nMaxConnectionCount)");
			return false;
		}

		if (!CTickerRunnable::Inst()->init())
		{
			PrintWarning("CTickerRunnable::Inst()->init()");
			return false;
		}

		if (!this->m_sNodeBaseInfo.szHost.empty())
		{
			if (!this->m_pGlobalBaseConnectionMgr->listen(this->m_sNodeBaseInfo.szHost, this->m_sNodeBaseInfo.nPort, false, "CBaseConnectionOtherNode", "", this->m_sNodeBaseInfo.nSendBufSize, this->m_sNodeBaseInfo.nRecvBufSize, nullptr))
			{
				PrintWarning("node listen error");
				return false;
			}
		}

		for (uint32_t i = 0; i < this->m_nLogicThreadCount; ++i)
		{
			CLogicRunnable* pLogicRunnable = new CLogicRunnable();
			if (!pLogicRunnable->init())
			{
				PrintWarning("pLogicRunnable->init()");
				return false;
			}

			this->m_vecLogicRunnable.push_back(pLogicRunnable);
		}

		if (!this->m_pCoreServiceMgr->onInit())
			return false;

		this->printNodeInfo();

		PrintInfo("CCoreApp::init");

		return true;
	}

	void CCoreApp::destroy()
	{
		PrintInfo("CCoreApp::destroy");

		this->unregisterTicker(&this->m_tickerQPS);
		CClassInfoMgr::Inst()->unRegisterClassInfo();
		CClassInfoMgr::Inst()->release();

		base::profiling::uninit();

		base::log::uninit();
#ifndef _WIN32
		// 删除pid文件
		remove(this->m_szPID.c_str());
#endif
	}

	void CCoreApp::doQuit()
	{
		PrintInfo("CCoreApp::doQuit");
		DebugAst(!this->m_nQuiting);

		this->m_nQuiting = true;

		SMessagePacket sMessagePacket;
		sMessagePacket.nType = eMCT_QUIT;
		sMessagePacket.pData = nullptr;
		sMessagePacket.nDataSize = 0;

		const std::vector<CCoreService*>& vecCoreService = this->m_pCoreServiceMgr->getCoreService();
		for (size_t i = 0; i < vecCoreService.size(); ++i)
		{
			vecCoreService[i]->getMessageQueue()->send(sMessagePacket);
		}
	}

	CBaseConnectionMgr* CCoreApp::getGlobalBaseConnectionMgr() const
	{
		return this->m_pGlobalBaseConnectionMgr;
	}

	CLogicMessageQueueMgr* CCoreApp::getLogicMessageQueueMgr() const
	{
		return this->m_pLogicMessageQueueMgr;
	}

	CLogicMessageQueue* CCoreApp::getGlobalLogicMessageQueue() const
	{
		return this->m_pGlobalLogicMessageQueue;
	}

	CCoreServiceMgr* CCoreApp::getCoreServiceMgr() const
	{
		return this->m_pCoreServiceMgr;
	}

	CServiceRegistryProxy* CCoreApp::getServiceRegistryProxy() const
	{
		return this->m_pServiceRegistryProxy;
	}

	uint32_t CCoreApp::getHeartbeatLimit() const
	{
		return this->m_nHeartbeatLimit;
	}

	uint32_t CCoreApp::getHeartbeatTime() const
	{
		return this->m_nHeartbeatTime;
	}

	uint32_t CCoreApp::getSamplingTime() const
	{
		return this->m_nSamplingTime;
	}

	void CCoreApp::onQPS(uint64_t nContext)
	{
		//PrintInfo("qps: {} net_queue_size: {} logic_queue_size: {}", this->m_nQPS, this->m_pNetMessageQueue->size(), this->m_pLogicMessageQueue->size());
		this->m_nQPS.store(0, std::memory_order_release);
	}

	void CCoreApp::incQPS()
	{
		this->m_nQPS.fetch_add(1, std::memory_order_relaxed);
	}

	uint32_t CCoreApp::getQPS() const
	{
		return this->m_nQPS.load(std::memory_order_acquire);
	}

	const SNodeBaseInfo& CCoreApp::getNodeBaseInfo() const
	{
		return this->m_sNodeBaseInfo;
	}

	uint32_t CCoreApp::getNodeID() const
	{
		return this->m_sNodeBaseInfo.nID;
	}

	uint32_t CCoreApp::getDefaultServiceInvokeTimeout() const
	{
		return this->m_nDefaultServiceInvokeTimeout;
	}

	uint32_t CCoreApp::getCoroutineStackSize() const
	{
		return this->m_nCoroutineStackSize;
	}

	uint32_t CCoreApp::getLogicThreadCount() const
	{
		return this->m_nLogicThreadCount;
	}

	void CCoreApp::printNodeInfo()
	{
		const SNodeBaseInfo& sNodeBaseInfo = CCoreApp::Inst()->getNodeBaseInfo();
		const std::vector<SServiceBaseInfo>& vecServiceBaseInfo = this->m_pCoreServiceMgr->getServiceBaseInfo();

		std::stringstream ss;
		ss << std::endl;
		ss << "############################################################" << std::endl;
		ss << "\tnode_name: " << sNodeBaseInfo.szName << " node_id: " << sNodeBaseInfo.nID << " node_group: " << sNodeBaseInfo.szGroup << std::endl;
		ss << std::endl;
		for (size_t i = 0; i < vecServiceBaseInfo.size(); ++i)
		{
			const SServiceBaseInfo& sServiceBaseInfo = vecServiceBaseInfo[i];
			ss << "\tservice_name: " << sServiceBaseInfo.szName << "\tservice_type: " << sServiceBaseInfo.szType << " \tservice_id: " << sServiceBaseInfo.nID << std::endl;
		}
		ss << std::endl;
		ss << "\twork_path: " << base::process_util::getCurrentWorkPath() << std::endl;
		ss << std::endl;
		ss << "\tlog_path: " << base::log::getPath() << std::endl;
		ss << std::endl;
		ss << "\tlog_async: " << base::log::isAsync() << std::endl;
		ss << std::endl;
		ss << "\tlog_debug: " << base::log::isDebug() << std::endl;
		ss << std::endl;
		ss << "\tprofiling: " << CCoreApp::Inst()->getSamplingTime() << std::endl;
		ss << "############################################################";

		base::log::save("INFO", true, "%s", ss.str().c_str());
	}
}