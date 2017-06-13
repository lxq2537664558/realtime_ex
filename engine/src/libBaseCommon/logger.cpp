#include "stdafx.h"
#include "logger.h"
#include "thread_base.h"
#include "base_time.h"
#include "base_function.h"
#include "spin_lock.h"

#include <string>
#include <map>
#include <list>
#include <iostream>

#ifndef _WIN32
#include <stdarg.h>
#endif

#define _LOG_FILE_SIZE		1024*1024*20
#define _FLUSH_LOG_TIME		5*60

#define _LOG_BUF_SIZE		1024
#define _LOG_FILE_NAME_SIZE 32

static uint32_t formatLog(char* szBuf, uint32_t nBufSize, const char* szPrefix, bool bGmtTime, const char* szFormat, va_list arg, uint8_t* nDay)
{
	if (nullptr == szFormat || szBuf == nullptr || szPrefix == nullptr)
		return 0;

	int64_t nCurTime = 0;
	base::STime sTime;
	if (bGmtTime)
	{
		nCurTime = base::getGmtTime();
		sTime = base::getGmtTimeTM(nCurTime);
	}
	else
	{
		nCurTime = base::getLocalTime();
		sTime = base::getLocalTimeTM(nCurTime);
	}

	char szTime[30] = { 0 };
	base::crt::snprintf(szTime, _countof(szTime), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
		sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec, nCurTime % 1000);

	if (nDay != nullptr)
		*nDay = (uint8_t)sTime.nDay;

	size_t nLen = 0;
	if (szPrefix[0] != 0)
		nLen = base::crt::snprintf(szBuf, nBufSize, "[%s\t] %s ", szPrefix, szTime);
	else
		nLen = base::crt::snprintf(szBuf, nBufSize, "%s ", szTime);
	
	nLen += base::crt::vsnprintf(szBuf + nLen, nBufSize - nLen, szFormat, arg);

	if (nLen >= _LOG_BUF_SIZE - 1)
	{
		// ½Ø¶Ï
		base::crt::strcpy(szBuf + nBufSize - 3, 3, "\r\n");
		nLen = _LOG_BUF_SIZE - 1;
	}
	else
	{
		base::crt::strcpy(szBuf + nLen, nBufSize - nLen, "\r\n");
		nLen = nLen + 2;
	}

	return (uint32_t)nLen;
}

static std::string formatLogName(const char* szPath, const char* szSuffix)
{
	base::STime sTime = base::getLocalTimeTM();
	char szBuf[MAX_PATH] = { 0 };
	if (szSuffix[0] == 0)
		base::crt::snprintf(szBuf, _countof(szBuf), "%s/%s.%4d_%02d_%02d_%02d_%02d_%02d.%d", szPath, base::getInstanceName(), sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec, base::getCurrentProcessID());
	else
		base::crt::snprintf(szBuf, _countof(szBuf), "%s/%s.%4d_%02d_%02d_%02d_%02d_%02d.%d_%s", szPath, base::getInstanceName(), sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec, base::getCurrentProcessID(), szSuffix);
	
	return szBuf;
}

struct SLogInfo
{
	char		szSuffix[_LOG_FILE_NAME_SIZE];
	uint32_t	nBufSize;
	uint8_t		bConsole;
	uint8_t		nDay;
	char		szBuf[1];

	SLogInfo()
		: nBufSize(0)
		, bConsole(false)
		, nDay(0)
	{
	}
};

class CLogger :
	public base::IRunnable
{
public:
	CLogger();
	virtual ~CLogger();

	bool				init(bool bAsync, bool bGmtTime, const char* szPath);
	void				uninit();
	void				pushLog(SLogInfo* pLogInfo);
	const char*			getPath() const;
	void				debug(bool bEnable);
	bool				isDebug() const;
	bool				isGmtTime() const;

private:
	virtual bool		onInit() { return true; }
	virtual void		onDestroy() { }
	virtual bool		onProcess();

	void				flushLog();
	void				saveLog(SLogInfo* pLogInfo);

private:
	struct SLogFileInfo
	{
		uint32_t	nNextFileIndex;
		size_t		nFileSize;
		FILE*		pFile;
		uint8_t		nLastLogDay;
	};

	char									m_szPath[MAX_PATH];
	std::map<std::string, SLogFileInfo*>	m_mapLogFileInfo;
	int64_t									m_nLastFlushTime;
	std::list<SLogInfo*>					m_listLogInfo;
	base::spin_lock							m_lock;
	base::CThreadBase*						m_pThreadBase;
	bool									m_bAsync;
	bool									m_bGmtTime;
	bool									m_bDebug;
};

CLogger::CLogger()
	: m_pThreadBase(nullptr)
	, m_bAsync(false)
	, m_bGmtTime(false)
	, m_bDebug(true)
{
	memset(this->m_szPath, 0, _countof(this->m_szPath));
	this->m_nLastFlushTime = base::getGmtTime() / 1000;
}

CLogger::~CLogger()
{
	for (auto iter = this->m_mapLogFileInfo.begin(); iter != this->m_mapLogFileInfo.end(); ++iter)
	{
		SLogFileInfo* pLogFileInfo = iter->second;
		if (pLogFileInfo != nullptr && pLogFileInfo->pFile != nullptr)
		{
			fflush(pLogFileInfo->pFile);
			fclose(pLogFileInfo->pFile);
			pLogFileInfo->pFile = nullptr;
		}

		delete pLogFileInfo;
	}

	this->m_mapLogFileInfo.clear();
}

bool CLogger::init(bool bAsync, bool bGmtTime, const char* szPath)
{
	if (szPath == nullptr)
		return false;

	std::string szTempPath = szPath;
	if (szTempPath.empty())
		return false;

	std::string::size_type pos = szTempPath.find(".");
	if (std::string::npos != pos && pos == 0)
	{
		szTempPath.replace(pos, 1, "");
		szTempPath.insert(0, base::getCurrentWorkPath());
	}
	
	while (std::string::npos != (pos = szTempPath.find("\\")))
	{
		szTempPath.replace(pos, 1, "/");
		pos += 1;
	}

	pos = szTempPath.find_last_of("/");
	if (pos != szTempPath.size() - 1)
		szTempPath.insert(szTempPath.size(), "/");

	pos = 0;
	while (std::string::npos != (pos = szTempPath.find("/", pos)))
	{
		std::string szSubPath = szTempPath.substr(0, pos);
		pos += 1;
		if (szSubPath.empty())
			continue;

		if (!base::createDir(szSubPath.c_str()))
			return false;
	}

	base::crt::snprintf(this->m_szPath, _countof(this->m_szPath), "%s%s", szTempPath.c_str(), base::getInstanceName());

	if (!base::createDir(this->m_szPath))
		return false;

	this->m_pThreadBase = base::CThreadBase::createNew(this);
	if (nullptr == this->m_pThreadBase)
		return false;

	this->m_bAsync = bAsync;
	this->m_bGmtTime = bGmtTime;

	return true;
}

void CLogger::uninit()
{
	if (this->m_pThreadBase == nullptr)
		return;

	this->m_pThreadBase->quit();
	this->m_pThreadBase->join();
	this->m_pThreadBase->release();
	this->m_pThreadBase = nullptr;
}

const char* CLogger::getPath() const
{
	return this->m_szPath;
}

void CLogger::debug(bool bEnable)
{
	this->m_bDebug = bEnable;
}

bool CLogger::isDebug() const
{
	return this->m_bDebug;
}

bool CLogger::isGmtTime() const
{
	return this->m_bGmtTime;
}

void CLogger::pushLog(SLogInfo* pLogInfo)
{
	if (nullptr == pLogInfo)
		return;

	if (this->m_bAsync)
	{
		this->m_lock.lock();
		this->m_listLogInfo.push_back(pLogInfo);
		this->m_lock.unlock();
	}
	else
	{
		this->saveLog(pLogInfo);
		char* szBuf = reinterpret_cast<char*>(pLogInfo);
		delete[](szBuf);
	}
}

bool CLogger::onProcess()
{
	std::list<SLogInfo*> listLogInfo;
	
	this->m_lock.lock();
	listLogInfo.splice(listLogInfo.end(), this->m_listLogInfo);
	this->m_lock.unlock();

	for (auto iter = listLogInfo.begin(); iter != listLogInfo.end(); ++iter)
	{
		SLogInfo* pLogInfo = *iter;
		if (pLogInfo == nullptr)
			continue;

		if (pLogInfo->nBufSize == 0)
		{
			this->flushLog();
			this->m_nLastFlushTime = base::getGmtTime();
		}
		else
		{
			this->saveLog(pLogInfo);
		}

		char* szBuf = reinterpret_cast<char*>(pLogInfo);
		delete [](szBuf);
	}

	int64_t nCurTime = base::getGmtTime();
	if (nCurTime - this->m_nLastFlushTime >= _FLUSH_LOG_TIME)
	{
		this->flushLog();
		this->m_nLastFlushTime = nCurTime;
	}

	base::sleep(100);
	return true;
}

void CLogger::flushLog()
{
	for (auto iter = this->m_mapLogFileInfo.begin(); iter != this->m_mapLogFileInfo.end(); ++iter)
	{
		SLogFileInfo* pLogFileInfo = iter->second;
		if (pLogFileInfo != nullptr && pLogFileInfo->pFile != nullptr)
			fflush(pLogFileInfo->pFile);
	}
}

void CLogger::saveLog(SLogInfo* pLogInfo)
{
	if (pLogInfo->bConsole)
		std::cout << pLogInfo->szBuf;

	SLogFileInfo* pLogFileInfo = nullptr;
	auto iter = this->m_mapLogFileInfo.find(pLogInfo->szSuffix);
	if (iter == this->m_mapLogFileInfo.end())
	{
		pLogFileInfo = new SLogFileInfo();
		pLogFileInfo->nNextFileIndex = 0;
		pLogFileInfo->nFileSize = 0;
		pLogFileInfo->nLastLogDay = pLogInfo->nDay;
		pLogFileInfo->pFile = nullptr;
		this->m_mapLogFileInfo[pLogInfo->szSuffix] = pLogFileInfo;
	}
	else
	{
		pLogFileInfo = iter->second;
	}

	if (pLogFileInfo->pFile == nullptr || pLogFileInfo->nFileSize >= _LOG_FILE_SIZE || pLogFileInfo->nLastLogDay != pLogInfo->nDay)
	{
		std::string szFileName = formatLogName(this->m_szPath, pLogInfo->szSuffix);
		char szFullFileName[MAX_PATH] = { 0 };
		base::crt::snprintf(szFullFileName, _countof(szFullFileName), "%s_%d.log", szFileName.c_str(), pLogFileInfo->nNextFileIndex);
#ifdef _WIN32
		FILE* pFile = _fsopen(szFullFileName, "w", _SH_DENYNO);
#else
		FILE* pFile = fopen(szFullFileName, "w");
#endif
		if (pFile == nullptr)
		{
			fprintf(stderr, "open log error\n");
			exit(0);
			return;
		}

		if (pLogFileInfo->pFile != nullptr)
		{
			fflush(pLogFileInfo->pFile);
			fclose(pLogFileInfo->pFile);
		}
		pLogFileInfo->pFile = pFile;
		pLogFileInfo->nLastLogDay = pLogInfo->nDay;
		pLogFileInfo->nFileSize = 0;

		++pLogFileInfo->nNextFileIndex;
	}

	size_t nSize = fwrite(pLogInfo->szBuf, 1, pLogInfo->nBufSize, pLogFileInfo->pFile);

	pLogFileInfo->nFileSize += nSize;
}

CLogger*	g_pLogger;
FILE*		g_pError;

namespace base
{
	bool initLog(bool bAsync, bool bGmtTime, const char* szPath)
	{
		g_pLogger = new CLogger();
		if (!g_pLogger->init(bAsync, bGmtTime, szPath))
			return false;

		std::string szFileName = formatLogName(g_pLogger->getPath(), "ERROR.log");
#ifdef _WIN32
		g_pError = _fsopen(szFileName.c_str(), "w", _SH_DENYNO);
#else
		g_pError = fopen(szFileName.c_str(), "w");
#endif
		if (g_pError == nullptr)
			return false;

		return true;
	}

	void uninitLog()
	{
		if (g_pError != nullptr)
			fclose(g_pError);

		if (g_pLogger != nullptr)
		{
			g_pLogger->uninit();
			delete g_pLogger;
		}
	}

	void saveLog(const char* szPrefix, bool bConsole, const char* szFormat, ...)
	{
		if (szFormat == nullptr || szPrefix == nullptr)
			return;

		uint8_t nDay = 0;
		char szBuf[4096] = { 0 };
		va_list arg;
		va_start(arg, szFormat);
		uint32_t nSize = formatLog(szBuf, _countof(szBuf), szPrefix, g_pLogger->isGmtTime(), szFormat, arg, &nDay);
		va_end(arg);
		if (nSize == 0)
			return;

		SLogInfo* pLogInfo = reinterpret_cast<SLogInfo*>(new char[sizeof(SLogInfo) + nSize + 1]);
		pLogInfo->nBufSize = nSize;
		pLogInfo->szSuffix[0] = 0;
		pLogInfo->bConsole = bConsole;
		pLogInfo->nDay = nDay;
		base::crt::strcpy(pLogInfo->szBuf, nSize + 1, szBuf);

		g_pLogger->pushLog(pLogInfo);
	}

	void saveLogEx(const char* szFileName, bool bConsole, const char* szFormat, ...)
	{
		if (szFormat == nullptr || szFileName == nullptr)
			return;

		uint8_t nDay = 0;
		char szBuf[4096] = { 0 };
		va_list arg;
		va_start(arg, szFormat);
		uint32_t nSize = formatLog(szBuf, _countof(szBuf), "", g_pLogger->isGmtTime(), szFormat, arg, &nDay);
		va_end(arg);
		if (nSize == 0)
			return;

		SLogInfo* pLogInfo = reinterpret_cast<SLogInfo*>(new char[sizeof(SLogInfo) + nSize + 1]);
		pLogInfo->nBufSize = nSize;
		base::crt::strcpy(pLogInfo->szSuffix, _LOG_FILE_NAME_SIZE, szFileName);
		pLogInfo->bConsole = bConsole;
		pLogInfo->nDay = nDay;
		base::crt::strcpy(pLogInfo->szBuf, nSize + 1, szBuf);

		g_pLogger->pushLog(pLogInfo);
	}

	void saveErrorLog(const char* szFormat, ...)
	{
		if (nullptr == szFormat)
			return;

		char szBuf[4096] = { 0 };
		uint32_t nSize = (uint32_t)base::getStackInfo(0, 10, szBuf, _countof(szBuf));

		va_list arg;
		va_start(arg, szFormat);
		nSize += formatLog(szBuf + nSize, _countof(szBuf) - nSize, "", g_pLogger->isGmtTime(), szFormat, arg, nullptr);
		va_end(arg);

		fwrite(szBuf, 1, nSize, g_pError);
		std::cout << szBuf;

		fflush(g_pError);
	}

	void flushLog()
	{
		if (g_pLogger == nullptr)
			return;

		SLogInfo* pLogInfo = new SLogInfo();
		pLogInfo->nBufSize = 0;
		g_pLogger->pushLog(pLogInfo);
	}

	void debugLog(bool bEnable)
	{
		if (g_pLogger == nullptr)
			return;

		g_pLogger->debug(bEnable);
	}

	bool isDebugLog()
	{
		if (g_pLogger == nullptr)
			return false;

		return g_pLogger->isDebug();
	}
}