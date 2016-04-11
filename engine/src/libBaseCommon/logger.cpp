#include "stdafx.h"
#include "logger.h"
#include "thread_base.h"
#include "base_time.h"
#include "base_function.h"
#include "safe_queue.h"
#include "spin_mutex.h"

#include <string>
#include <map>
#include <iostream>

#ifndef _WIN32
#include <stdarg.h>
#endif

#define _LOG_FILE_SIZE		1024*1024*20
#define _FLUSH_LOG_TIME		5*60

#define _LOG_BUF_SIZE		1024
#define _LOG_FILE_NAME_SIZE 32

static uint32_t formatLog(char* szBuf, uint32_t nBufSize, const char* szSection, const char* szFormat, va_list arg)
{
	if (nullptr == szFormat || szBuf == nullptr || szSection == nullptr)
		return 0;

	int64_t nCurTime = base::getLocalTime();
	char szTime[20] = { 0 };
	base::formatLocalTime(szTime, _countof(szTime), nCurTime);

	if (base::crt::snprintf(szBuf, nBufSize, "%s.%03d ", szTime, (uint32_t)(nCurTime % 1000)) < 0)
		return 0;

	if (szSection[0] != 0 && base::crt::snprintf(szBuf, nBufSize, "[%s] ", szTime, szSection) < 0)
		return 0;
	
	size_t nLen = base::crt::strnlen(szBuf, nBufSize);
	if (base::crt::vsnprintf(szBuf + nLen, nBufSize - nLen, szFormat, arg) < 0)
		return 0;

	nLen = base::crt::strnlen(szBuf, nBufSize);
	if (nLen >= _LOG_BUF_SIZE - 1)
	{
		// ½Ø¶Ï
		base::crt::strncpy(szBuf + nBufSize - 3, 3, "\r\n", _TRUNCATE);
		nLen = _LOG_BUF_SIZE - 1;
	}
	else
	{
		base::crt::strncpy(szBuf + nLen, nBufSize - nLen, "\r\n", _TRUNCATE);
		nLen = nLen + 2;
	}

	return (uint32_t)nLen;
}

struct SLogInfo
{
	char		szFileName[_LOG_FILE_NAME_SIZE];
	char		szBuf[_LOG_BUF_SIZE];
	uint16_t	nBufSize;
	bool		bConsole;

	SLogInfo()
		: nBufSize(0)
		, bConsole(false)
	{

	}

	SLogInfo& operator = (const SLogInfo& lhs)
	{
		if (this == &lhs)
			return *this;

		if (lhs.nBufSize > _LOG_BUF_SIZE)
			return *this;

		base::crt::strncpy(this->szBuf, _LOG_BUF_SIZE, lhs.szBuf, _TRUNCATE);
		this->nBufSize = lhs.nBufSize;

		base::crt::strncpy(this->szFileName, _LOG_FILE_NAME_SIZE, lhs.szFileName, _TRUNCATE);
		this->bConsole = lhs.bConsole;

		return *this;
	}
};

class CLogger :
	public base::IRunnable
{
public:
	CLogger();
	virtual ~CLogger();

	bool				init(bool bAsync);
	void				uninit();
	void				pushLog(const SLogInfo& sLogInfo);

private:
	virtual bool		onInit() { return true; }
	virtual void		onDestroy() { }
	virtual bool		onProcess();

	void				flushLog();
	void				saveLog(const char* szFileName, bool bConsole, const char* szBuf, uint32_t nBufSize);

private:
	struct SLogFileInfo
	{
		std::string	szFileName;
		uint32_t	nFileIndex;
		size_t		nFileSize;
		FILE*		pFile;
	};

	char									m_szPath[MAX_PATH];
	std::map<std::string, SLogFileInfo*>	m_mapLogFileInfo;
	int64_t									m_nLastFlushTime;
	base::safe_queue<SLogInfo, 10000>		m_logQueue;
	base::spin_mutex						m_lock;
	base::CThreadBase*						m_pThreadBase;
	bool									m_bAsync;
};

CLogger::CLogger()
	: m_pThreadBase(nullptr)
	, m_bAsync(false)
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

bool CLogger::init(bool bAsync)
{
	base::crt::snprintf(this->m_szPath, _countof(this->m_szPath), "%s/log", base::getCurrentWorkPath());

	if (!base::createDir(this->m_szPath))
		return false;

	this->m_pThreadBase = base::CThreadBase::createNew(this);
	if (nullptr == this->m_pThreadBase)
		return false;

	this->m_bAsync = bAsync;

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

void CLogger::pushLog(const SLogInfo& sLogInfo)
{
	if (this->m_bAsync)
	{
		std::lock_guard<base::spin_mutex> guard(this->m_lock);
		this->m_logQueue.push(sLogInfo);
	}
	else
	{
		this->saveLog(sLogInfo.szFileName, sLogInfo.bConsole, sLogInfo.szBuf, sLogInfo.nBufSize);
	}
}

bool CLogger::onProcess()
{
	while (true)
	{
		SLogInfo sLogInfo;
		if (!this->m_logQueue.pop(sLogInfo))
			break;

		if (sLogInfo.nBufSize == 0)
		{
			this->flushLog();
			this->m_nLastFlushTime = base::getGmtTime();
		}
		else
		{
			this->saveLog(sLogInfo.szFileName, sLogInfo.bConsole, sLogInfo.szBuf, sLogInfo.nBufSize);
		}
	}

	int64_t nCurTime = base::getGmtTime();
	if (nCurTime - this->m_nLastFlushTime >= _FLUSH_LOG_TIME)
	{
		this->flushLog();
		this->m_nLastFlushTime = nCurTime;
	}

	base::sleep(1);
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

void CLogger::saveLog(const char* szFileName, bool bConsole, const char* szBuf, uint32_t nBufSize)
{
	if (bConsole)
		std::cout << szBuf;

	SLogFileInfo* pLogFileInfo = nullptr;
	auto iter = this->m_mapLogFileInfo.find(szFileName);
	if (iter == this->m_mapLogFileInfo.end())
	{
		base::STime sTime = base::getLocalTimeTM();
		pLogFileInfo = new SLogFileInfo();
		pLogFileInfo->nFileIndex = 0;
		pLogFileInfo->nFileSize = 0;
		char szFullFileName[MAX_PATH] = { 0 };
		pLogFileInfo->pFile = nullptr;
		if (szFileName[0] == 0)
			base::crt::snprintf(szFullFileName, _countof(szFullFileName), "%s/%s.%d.%4d_%02d_%02d_%02d_%02d_%02d", this->m_szPath, base::getInstanceName(), base::getCurrentProcessID(), sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec);
		else
			base::crt::snprintf(szFullFileName, _countof(szFullFileName), "%s/%s_%s.%d.%4d_%02d_%02d_%02d_%02d_%02d", this->m_szPath, base::getInstanceName(), szFileName, base::getCurrentProcessID(), sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec);
		pLogFileInfo->szFileName = szFullFileName;
		base::crt::snprintf(szFullFileName, _countof(szFullFileName), "%s_%d.log", pLogFileInfo->szFileName.c_str(), pLogFileInfo->nFileIndex);
#ifdef _WIN32
		if ((pLogFileInfo->pFile = _fsopen(szFullFileName, "w", _SH_DENYNO)) == nullptr)
#else
		if ((pLogFileInfo->pFile = fopen(szFullFileName, "w")) == nullptr)
#endif
		{
			delete pLogFileInfo;
			return;
		}
		this->m_mapLogFileInfo[szFileName] = pLogFileInfo;
	}
	else
	{
		pLogFileInfo = iter->second;
	}

	size_t nSize = fwrite(szBuf, 1, nBufSize, pLogFileInfo->pFile);

	pLogFileInfo->nFileSize += nSize;

	if (pLogFileInfo->nFileSize >= _LOG_FILE_SIZE)
	{
		++pLogFileInfo->nFileIndex;
		pLogFileInfo->nFileSize = 0;
		char szFullFileName[MAX_PATH] = { 0 };
		base::crt::snprintf(szFullFileName, _countof(szFullFileName), "%s_%d.log", pLogFileInfo->szFileName.c_str(), pLogFileInfo->nFileIndex);
		FILE* pFile = nullptr;
#ifdef _WIN32
		if ((pFile = _fsopen(szFullFileName, "w", _SH_DENYNO)) != nullptr)
#else
		if ((pFile = fopen(szFullFileName, "w")) != nullptr)
#endif
		{
			fflush(pLogFileInfo->pFile);
			fclose(pLogFileInfo->pFile);
			pLogFileInfo->pFile = pFile;
		}
	}
}

CLogger*	g_pLogger;
FILE*		g_pError;

namespace base
{
	bool initLog(bool bAsync)
	{
		g_pLogger = new CLogger();
		if (!g_pLogger->init(bAsync))
			return false;

		char szName[MAX_PATH] = { 0 };
		base::STime sTime = base::getLocalTimeTM();
		base::crt::snprintf(szName, _countof(szName), "%s/log/%s.%d.ERROR_%4d_%02d_%02d_%02d_%02d_%02d.log", base::getCurrentWorkPath(), base::getInstanceName(), base::getCurrentProcessID(), sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec);
#ifdef _WIN32
		if ((g_pError = _fsopen(szName, "w", _SH_DENYNO)) == nullptr)
#else
		if ((g_pError = fopen(szName, "w")) == nullptr)
#endif
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

	void saveLog(const char* szSection, bool bConsole, const char* szFormat, ...)
	{
		if (szFormat == nullptr || szSection == nullptr)
			return;

		SLogInfo sLogInfo;
		va_list arg;
		va_start(arg, szFormat);
		uint32_t nSize = formatLog(sLogInfo.szBuf, _countof(sLogInfo.szBuf), szSection, szFormat, arg);
		va_end(arg);
		if (nSize == 0)
			return;

		sLogInfo.nBufSize = (uint16_t)nSize;
		sLogInfo.szFileName[0] = 0;
		sLogInfo.bConsole = bConsole;

		g_pLogger->pushLog(sLogInfo);
	}

	void saveLogEx(const char* szFileName, bool bConsole, const char* szFormat, ...)
	{
		if (szFormat == nullptr || szFileName == nullptr)
			return;

		SLogInfo sLogInfo;
		va_list arg;
		va_start(arg, szFormat);
		uint32_t nSize = formatLog(sLogInfo.szBuf, _countof(sLogInfo.szBuf), "", szFormat, arg);
		va_end(arg);
		if (nSize == 0)
			return;

		sLogInfo.nBufSize = (uint16_t)nSize;
		base::crt::strncpy(sLogInfo.szFileName, _LOG_FILE_NAME_SIZE, szFileName, _TRUNCATE);
		sLogInfo.bConsole = bConsole;

		g_pLogger->pushLog(sLogInfo);
	}

	void saveErrorLog(const char* szFormat, ...)
	{
		if (nullptr == szFormat)
			return;

		char szBuf[4096] = { 0 };
		uint32_t nSize = (uint32_t)base::getStackInfo(0, 10, szBuf, _countof(szBuf));

		va_list arg;
		va_start(arg, szFormat);
		nSize += formatLog(szBuf + nSize, _countof(szBuf) - nSize, "", szFormat, arg);
		va_end(arg);

		fwrite(szBuf, 1, nSize, g_pError);
		std::cout << szBuf;

		fflush(g_pError);
	}

	void flushLog()
	{
		if (g_pLogger == nullptr)
			return;

		SLogInfo sLogInfo;
		sLogInfo.nBufSize = 0;
		g_pLogger->pushLog(sLogInfo);
	}
}