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

#include "base_app_impl.h"
#include "base_app.h"
#include "base_object.h"
#include "ticker_mgr.h"
#include "monitor.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "message_command.h"
#include "net_runnable.h"

#include "tinyxml2/tinyxml2.h"

#define _DEFAULT_HEARTBEAT_LIMIT	3
#define _DEFAULT_HEARTBEAT_TIME		10
#define _MAIN_CO_STACK_SIZE			10*1024*1024
#define _DEFAULT_SAMPLING_TIME		1000

#ifndef _WIN32
// 环境变量
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
	CBaseAppImpl::CBaseAppImpl()
		: m_pTickerMgr(nullptr)
		, m_pMessageQueue(nullptr)
		, m_pBaseConnectionMgr(nullptr)
		, m_pCoroutineMgr(nullptr)
		, m_nRunState(eARS_Start)
		, m_bMarkQuit(false)
		, m_nTotalTime(0)
		, m_nCycleCount(0)
		, m_nHeartbeatLimit(0)
		, m_nHeartbeatTime(0)
		, m_nQPS(0)
	{
	}

	CBaseAppImpl::~CBaseAppImpl()
	{

	}

	bool CBaseAppImpl::run(int32_t argc, char** argv, const char* szConfig)
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

	void CBaseAppImpl::registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		this->m_pTickerMgr->registerTicker(pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CBaseAppImpl::unregisterTicker(CTicker* pTicker)
	{
		this->m_pTickerMgr->unregisterTicker(pTicker);
	}

	int64_t CBaseAppImpl::getLogicTime() const
	{
		return this->m_pTickerMgr->getLogicTime();
	}

	CBaseConnectionMgr* CBaseAppImpl::getBaseConnectionMgr() const
	{
		return this->m_pBaseConnectionMgr;
	}

	CCoroutineMgr* CBaseAppImpl::getCoroutineMgr() const
	{
		return this->m_pCoroutineMgr;
	}

	const std::string& CBaseAppImpl::getConfigFileName() const
	{
		return this->m_szConfig;
	}

	base::CWriteBuf& CBaseAppImpl::getWriteBuf() const
	{
		const_cast<base::CWriteBuf&>(this->m_writeBuf).clear();
		return const_cast<base::CWriteBuf&>(this->m_writeBuf);
	}

	bool CBaseAppImpl::onInit()
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
			if (CBaseAppImpl::Inst()->m_nRunState != eARS_Normal)
				return;

			PrintInfo("server start quit");
			CBaseAppImpl::Inst()->m_nRunState = eARS_Quitting;
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

		tinyxml2::XMLElement* pLogXML = pBaseInfoXML->FirstChildElement("log");
		if (pLogXML == nullptr)
		{
			fprintf(stderr, "pLogXML == nullptr\n");
			return false;
		}

		if (!base::initLog(pLogXML->IntAttribute("sync") != 0, pLogXML->Attribute("path")))
		{
			fprintf(stderr, "init log error\n");
			return false;
		}

		base::enableDebugLog(pLogXML->IntAttribute("debug") != 0);

		bool bProfiling = false;
		tinyxml2::XMLElement* pProfilingXML = pBaseInfoXML->FirstChildElement("profiling");
		if (pProfilingXML != nullptr)
		{
			this->m_nSamplingTime = (uint32_t)pProfilingXML->IntAttribute("sampling_time");
			bProfiling = true;
		}
		else
		{
			this->m_nSamplingTime = _DEFAULT_SAMPLING_TIME;
		}
		
		if (!base::initProfiling(bProfiling))
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

		this->m_pMessageQueue = new CMessageQueue();
		if (!this->m_pMessageQueue->init(true))
		{
			PrintWarning("this->m_pMessageQueue->init error");
			return false;
		}

		this->m_pCoroutineMgr = new CCoroutineMgr();
		if (!this->m_pCoroutineMgr->init(_MAIN_CO_STACK_SIZE))
		{
			PrintWarning("this->m_pCoroutineMgr->init(_MAIN_CO_STACK_SIZE)");
			return false;
		}
		PrintInfo("main coroutine id: "UINT64FMT, coroutine::getCurrentID());

		this->m_pTickerMgr = new core::CTickerMgr();
		uint32_t nMaxConnectionCount = (uint32_t)pBaseInfoXML->IntAttribute("connections");
		this->m_pBaseConnectionMgr = new CBaseConnectionMgr();
		if (!this->m_pBaseConnectionMgr->init())
		{
			PrintWarning("this->m_pCoreConnectionMgr->init(nMaxConnectionCount)");
			return false;
		}

		if (!CNetRunnable::Inst()->init(nMaxConnectionCount))
		{
			PrintWarning("CNetRunnable::Inst()->init(nMaxConnectionCount)");
			return false;
		}
		
		// 加载服务连接心跳信息
		tinyxml2::XMLElement* pHeartbeatXML = pBaseInfoXML->FirstChildElement("heartbeat");
		if (nullptr != pHeartbeatXML)
		{
			this->m_nHeartbeatLimit = pHeartbeatXML->UnsignedAttribute("heartbeat_limit");
			this->m_nHeartbeatTime = pHeartbeatXML->UnsignedAttribute("heartbeat_time");
		}
		else
		{
			this->m_nHeartbeatLimit = _DEFAULT_HEARTBEAT_LIMIT;
			this->m_nHeartbeatTime = _DEFAULT_HEARTBEAT_TIME;
		}

		this->m_tickerQPS.setCallback(std::bind(&CBaseAppImpl::onQPS, this, std::placeholders::_1));
		this->registerTicker(&this->m_tickerQPS, 1000, 1000, 0);

		SAFE_DELETE(pConfigXML);

		this->m_nRunState = eARS_Normal;

		PrintInfo("CCoreApp::onInit");

		return CBaseApp::Inst()->onInit();
	}

	void CBaseAppImpl::onDestroy()
	{
		CBaseApp::Inst()->onDestroy();

		CNetRunnable::Inst()->join();
		CNetRunnable::Inst()->release();

		PrintInfo("CCoreApp::onDestroy");

		SAFE_DELETE(this->m_pBaseConnectionMgr);
		SAFE_DELETE(this->m_pMessageQueue);
		SAFE_DELETE(this->m_pTickerMgr);
		SAFE_DELETE(this->m_pCoroutineMgr);

		CBaseObject::unRegistClassInfo();

		uninitMonitor();
		base::uninitProfiling();
		base::uninitLog();

#ifndef _WIN32
		// 删除pid文件
		remove(this->m_szPID.c_str());
#endif
	}

	bool CBaseAppImpl::onProcess()
	{
		int64_t nBeginTime = base::getProcessPassTime();

		static std::vector<SMessagePacket> vecMessagePacket;
		this->m_pMessageQueue->popMessagePacket(vecMessagePacket);

		for (auto iter = vecMessagePacket.begin(); iter != vecMessagePacket.end(); ++iter)
		{
			const SMessagePacket& sMessagePacket = *iter;

			switch (sMessagePacket.nType)
			{
				case eMCT_NOTIFY_SOCKET_CONNECT:
				{
					SMCT_NOTIFY_SOCKET_CONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
					 PrintWarning("context == nullptr type: eMCT_NOTIFY_SOCKET_CONNECT");
					 continue;
					}
					this->m_pBaseConnectionMgr->onConnect(pContext->nSocketID, pContext->szContext, pContext->nType, pContext->sLocalAddr, pContext->sRemoteAddr);
					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_NOTIFY_SOCKET_DISCONNECT:
				{
					SMCT_NOTIFY_SOCKET_DISCONNECT* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_DISCONNECT*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMPT_NOTIFY_SOCKET_DISCONNECT");
						continue;
					}
					this->m_pBaseConnectionMgr->onDisconnect(pContext->nSocketID);
					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_NOTIFY_SOCKET_CONNECT_REFUSE:
				{
					SMCT_NOTIFY_SOCKET_CONNECT_REFUSE* pContext = reinterpret_cast<SMCT_NOTIFY_SOCKET_CONNECT_REFUSE*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_NOTIFY_SOCKET_CONNECT_REFUSE");
						continue;
					}
					this->m_pBaseConnectionMgr->onConnectRefuse(pContext->szContext);
					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_RECV_SOCKET_DATA:
				{
					SMCT_RECV_SOCKET_DATA* pContext = reinterpret_cast<SMCT_RECV_SOCKET_DATA*>(sMessagePacket.pData);
					if (pContext == nullptr)
					{
						PrintWarning("context == nullptr type: eMCT_RECV_SOCKET_DATA");
						continue;
					}
					CBaseConnection* pBaseConnection = this->m_pBaseConnectionMgr->getBaseConnectionByID(pContext->nSocketID);
					if (pBaseConnection == nullptr)
					{
						PrintWarning("pContext->pBaseConnection == nullptr type: eMCT_RECV_SOCKET_DATA socket_id: %d", pContext->nSocketID);
						continue;
					}
					if (pBaseConnection->onDispatch(pContext->nMessageType, pContext->pData, pContext->nDataSize))
					{
						char* pData = reinterpret_cast<char*>(pContext->pData);
						SAFE_DELETE_ARRAY(pData);
					}
					SAFE_DELETE(pContext);
				}
				break;

			case eMCT_TIMER:
				{
					PROFILING_BEGIN(CBaseApp::Inst()->onBeforeFrame())
					CBaseApp::Inst()->onBeforeFrame();
					PROFILING_END(CBaseApp::Inst()->onBeforeFrame())

					PROFILING_BEGIN(this->m_pTickerMgr->update)
					this->m_pTickerMgr->update();
					PROFILING_END(this->m_pTickerMgr->update)

					PROFILING_BEGIN(CBaseApp::Inst()->onAfterFrame)
					CBaseApp::Inst()->onAfterFrame();
					PROFILING_END(CBaseApp::Inst()->onAfterFrame)

					++this->m_nCycleCount;
				}
				break;

			default:
				{
					PrintWarning("invalid type: %d", sMessagePacket.nType);
				}
			}
		}

		if (this->m_nRunState == eARS_Quitting && !this->m_bMarkQuit)
		{
			this->m_bMarkQuit = true;

			PrintInfo("CCoreApp::onQuit");
			
			base::flushLog();
			CBaseApp::Inst()->onQuit();
		}
		if (this->m_nRunState == eARS_Quit)
			return false;

		int64_t nEndTime = base::getProcessPassTime();
		this->m_nTotalTime = this->m_nTotalTime + (uint32_t)(nEndTime - nBeginTime);

		if (this->m_nTotalTime / 1000 >= this->m_nSamplingTime)
		{
			this->onAnalyze();
#ifdef __PROFILING_OPEN
			base::profiling(this->m_nTotalTime);
#endif
			this->m_nTotalTime = 0;
		}
		return true;
	}

	CMessageQueue* CBaseAppImpl::getMessageQueue() const
	{
		return this->m_pMessageQueue;
	}

	void CBaseAppImpl::onAnalyze()
	{
	}

	void CBaseAppImpl::doQuit()
	{
		PrintInfo("CCoreApp::doQuit");
		DebugAst(this->m_nRunState == eARS_Quitting);

		this->m_nRunState = eARS_Quit;
	}

	uint32_t CBaseAppImpl::getHeartbeatLimit() const
	{
		return this->m_nHeartbeatLimit;
	}

	uint32_t CBaseAppImpl::getHeartbeatTime() const
	{
		return this->m_nHeartbeatTime;
	}

	void CBaseAppImpl::onQPS(uint64_t nContext)
	{
		PrintInfo("QPS: %u", this->m_nQPS);
		this->m_nQPS = 0;
	}

	void CBaseAppImpl::incQPS()
	{
		++this->m_nQPS;
	}

}