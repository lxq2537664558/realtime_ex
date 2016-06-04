#include "stdafx.h"
#include "logger.h"
#include "thread_base.h"
#include "base_time.h"
#include "base_function.h"
#include "spin_mutex.h"

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
	uint32_t	nBufSize;
	bool		bConsole;
	char		szBuf[1];

	SLogInfo()
		: nBufSize(0)
		, bConsole(false)
	{
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
	void				pushLog(SLogInfo* pLogInfo);

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
	std::list<SLogInfo*>					m_listLogInfo;
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
		this->saveLog(pLogInfo->szFileName, pLogInfo->bConsole, pLogInfo->szBuf, pLogInfo->nBufSize);
	}
}

bool CLogger::onProcess()
{
	std::list<SLogInfo*> listLogInfo;
	
	this->m_lock.lock();
	listLogInfo = this->m_listLogInfo;
	this->m_listLogInfo.clear();
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
			this->saveLog(pLogInfo->szFileName, pLogInfo->bConsole, pLogInfo->szBuf, pLogInfo->nBufSize);
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
		pLogFileInfo->pFile = _fsopen(szFullFileName, "w", _SH_DENYNO);
#else
		pLogFileInfo->pFile = fopen(szFullFileName, "w");
#endif
		if (pLogFileInfo->pFile == nullptr)
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
		pFile = _fsopen(szFullFileName, "w", _SH_DENYNO);
#else
		pFile = fopen(szFullFileName, "w");
#endif
		if (pFile == nullptr)
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
		g_pError = _fsopen(szName, "w", _SH_DENYNO);
#else
		g_pError = fopen(szName, "w");
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

	void saveLog(const char* szSection, bool bConsole, const char* szFormat, ...)
	{
		if (szFormat == nullptr || szSection == nullptr)
			return;

		char szBuf[4096] = { 0 };
		va_list arg;
		va_start(arg, szFormat);
		uint32_t nSize = formatLog(szBuf, _countof(szBuf), szSection, szFormat, arg);
		va_end(arg);
		if (nSize == 0)
			return;

		SLogInfo* pLogInfo = reinterpret_cast<SLogInfo*>(new char[sizeof(SLogInfo) + nSize + 1]);
		pLogInfo->nBufSize = nSize;
		pLogInfo->szFileName[0] = 0;
		pLogInfo->bConsole = bConsole;
		base::crt::strncpy(pLogInfo->szBuf, nSize + 1, szBuf, _TRUNCATE);

		g_pLogger->pushLog(pLogInfo);
	}

	void saveLogEx(const char* szFileName, bool bConsole, const char* szFormat, ...)
	{
		if (szFormat == nullptr || szFileName == nullptr)
			return;

		char szBuf[4096] = { 0 };
		va_list arg;
		va_start(arg, szFormat);
		uint32_t nSize = formatLog(szBuf, _countof(szBuf), "", szFormat, arg);
		va_end(arg);
		if (nSize == 0)
			return;

		SLogInfo* pLogInfo = reinterpret_cast<SLogInfo*>(new char[sizeof(SLogInfo) + nSize + 1]);
		pLogInfo->nBufSize = nSize;
		base::crt::strncpy(pLogInfo->szFileName, _LOG_FILE_NAME_SIZE, szFileName, _TRUNCATE);
		pLogInfo->bConsole = bConsole;
		base::crt::strncpy(pLogInfo->szBuf, nSize + 1, szBuf, _TRUNCATE);

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

		SLogInfo* pLogInfo = new SLogInfo();
		pLogInfo->nBufSize = 0;
		g_pLogger->pushLog(pLogInfo);
	}
}