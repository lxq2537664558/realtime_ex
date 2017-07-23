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
#include <algorithm>

#include "libBaseCommon/base_function.h"
#include "libBaseCommon/exception_handler.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/logger.h"

#include "core_app.h"
#include "base_app.h"
#include "base_object.h"
#include "monitor.h"
#include "base_connection_mgr.h"
#include "base_connection.h"
#include "message_command.h"
#include "class_info_mgr.h"

#include "tinyxml2/tinyxml2.h"
#include "service_base_impl.h"

#define _DEFAULT_HEARTBEAT_LIMIT	3
#define _DEFAULT_HEARTBEAT_TIME		10
#define _DEFAULT_SAMPLING_TIME		1000
#define _DEFAULT_INVOKE_TIMEOUT	30*1000

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
	CCoreApp::CCoreApp()
		: m_writeBuf(UINT16_MAX)
		, m_nQuiting(0)
		, m_nTotalSamplingTime(0)
		, m_nCycleCount(0)
		, m_nHeartbeatLimit(_DEFAULT_HEARTBEAT_LIMIT)
		, m_nHeartbeatTime(_DEFAULT_HEARTBEAT_TIME)
		, m_nSamplingTime(_DEFAULT_SAMPLING_TIME)
		, m_nInvokeTimeout(_DEFAULT_INVOKE_TIMEOUT)
		, m_nQPS(0)
		, m_pLogicRunnable(nullptr)
		, m_pNetRunnable(nullptr)
		, m_pTickerRunnable(nullptr)
	{

	}

	CCoreApp::~CCoreApp()
	{

	}

	bool CCoreApp::run(const std::string& szInstanceName, const std::string& szConfig)
	{
		this->m_szConfig = szConfig;

		base::setInstanceName(szInstanceName.c_str());

		base::initProcessExceptionHander();
		base::initThreadExceptionHander();

		if (!this->init())
			return false;

		this->m_pLogicRunnable->join();
		this->m_pTickerRunnable->join();
		this->m_pNetRunnable->join();

		this->destroy();

		base::uninitProcessExceptionHander();

		return true;
	}

	void CCoreApp::registerTicker(uint8_t nType, uint32_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext)
	{
		this->m_pTickerRunnable->registerTicker(nType, nFromServiceID, nFromActorID, pTicker, nStartTime, nIntervalTime, nContext);
	}

	void CCoreApp::unregisterTicker(CTicker* pTicker)
	{
		this->m_pTickerRunnable->unregisterTicker(pTicker);
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

	bool CCoreApp::init()
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
			CCoreApp::Inst()->doQuit();
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

		if (!base::initLog(pLogXML->IntAttribute("async") != 0, false, pLogXML->Attribute("path")))
		{
			fprintf(stderr, "init log error\n");
			return false;
		}

		base::debugLog(pLogXML->IntAttribute("debug") != 0);

		this->m_pTickerRunnable = new CTickerRunnable();
		this->m_pNetRunnable = new CNetRunnable();
		this->m_pLogicRunnable = new CLogicRunnable();

		bool bProfiling = false;
		tinyxml2::XMLElement* pProfilingXML = pBaseInfoXML->FirstChildElement("profiling");
		if (pProfilingXML != nullptr)
		{
			this->m_nSamplingTime = (uint32_t)pProfilingXML->IntAttribute("sampling_time");
			bProfiling = true;
		}
		
		if (!base::initProfiling(bProfiling))
		{
			PrintWarning("base::initProfiling()");
			return false;
		}

		if (!initMonitor())
		{
			PrintWarning("initMonitor()");
			return false;
		}

		uint32_t nMaxConnectionCount = (uint32_t)pBaseInfoXML->IntAttribute("connections");

		tinyxml2::XMLElement* pNodeInfoXML = pRootXML->FirstChildElement("node_info");
		if (pNodeInfoXML == nullptr)
		{
			PrintWarning("pNodeInfoXML == nullptr");
			return false;
		}

		uint32_t nID = pNodeInfoXML->UnsignedAttribute("node_id");
		if (nID > UINT16_MAX)
		{
			PrintWarning("too big node id: %d", nID);
			return false;
		}
		// 加载节点基本信息
		this->m_sNodeBaseInfo.nID = (uint16_t)nID;
		this->m_sNodeBaseInfo.szName = pNodeInfoXML->Attribute("node_name");
		if (pNodeInfoXML->Attribute("host") != nullptr)
			this->m_sNodeBaseInfo.szHost = pNodeInfoXML->Attribute("host");
		this->m_sNodeBaseInfo.nPort = (uint16_t)pNodeInfoXML->UnsignedAttribute("port");
		this->m_sNodeBaseInfo.nRecvBufSize = pNodeInfoXML->UnsignedAttribute("recv_buf_size");
		this->m_sNodeBaseInfo.nSendBufSize = pNodeInfoXML->UnsignedAttribute("send_buf_size");
		
		// 加载服务连接心跳信息
		tinyxml2::XMLElement* pHeartbeatXML = pBaseInfoXML->FirstChildElement("heartbeat");
		if (nullptr != pHeartbeatXML)
		{
			this->m_nHeartbeatLimit = pHeartbeatXML->UnsignedAttribute("heartbeat_limit");
			this->m_nHeartbeatTime = pHeartbeatXML->UnsignedAttribute("heartbeat_time");
		}

		this->m_tickerQPS.setCallback(std::bind(&CCoreApp::onQPS, this, std::placeholders::_1));
		this->registerTicker(CTicker::eTT_Service, 0, 0, &this->m_tickerQPS, 1000, 1000, 0);

		if (!this->m_pNetRunnable->init(nMaxConnectionCount))
		{
			PrintWarning("this->m_pNetRunnable->init(nMaxConnectionCount)");
			return false;
		}

		if (!this->m_pLogicRunnable->init(pRootXML))
		{
			PrintWarning("this->m_pLogicRunnable->init(pRootXML)");
			return false;
		}

		if (!this->m_pTickerRunnable->init())
		{
			PrintWarning("this->m_pTickerRunnable->init()");
			return false;
		}

		SAFE_DELETE(pConfigXML);

		PrintInfo("CCoreApp::init");

		return true;
	}

	void CCoreApp::destroy()
	{
		PrintInfo("CCoreApp::destroy");

		SAFE_DELETE(this->m_pLogicRunnable);
		SAFE_DELETE(this->m_pTickerRunnable);
		SAFE_DELETE(this->m_pNetRunnable);

		CClassInfoMgr::Inst()->unRegisterClassInfo();
		
		uninitMonitor();
		base::uninitProfiling();
		base::uninitLog();

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

		this->m_pLogicRunnable->getMessageQueue()->send(sMessagePacket);
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
		this->m_nQPS = 0;
	}

	void CCoreApp::incQPS()
	{
		++this->m_nQPS;
	}

	uint32_t CCoreApp::getQPS() const
	{
		return this->m_nQPS;
	}

	const SNodeBaseInfo& CCoreApp::getNodeBaseInfo() const
	{
		return this->m_sNodeBaseInfo;
	}

	uint32_t CCoreApp::getNodeID() const
	{
		return this->m_sNodeBaseInfo.nID;
	}

	CNetRunnable* CCoreApp::getNetRunnable() const
	{
		return this->m_pNetRunnable;
	}

	CTickerRunnable* CCoreApp::getTickerRunnable() const
	{
		return this->m_pTickerRunnable;
	}

	CLogicRunnable* CCoreApp::getLogicRunnable() const
	{
		return this->m_pLogicRunnable;
	}

	uint32_t CCoreApp::getInvokeTimeout() const
	{
		return this->m_nInvokeTimeout;
	}

}