#include "stdafx.h"
#include "logger.h"
#include "thread_base.h"
#include "time_util.h"
#include "function_util.h"
#include "spin_lock.h"
#include "file_util.h"
#include "process_util.h"
#include "exception_handler.h"

#include <string>
#include <map>
#include <vector>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdarg.h>
#endif

#define _LOG_FILE_SIZE		1024*1024*20
#define _FLUSH_LOG_TIME		10*1000

#define _LOG_BUF_SIZE		4096
#define _LOG_FILE_NAME_SIZE 32

namespace
{
	uint32_t formatLog(char* szBuf, uint32_t nBufSize, const char* szPrefix, bool bGmtTime, const char* szFormat, va_list arg, uint8_t* nDay)
	{
		if (nullptr == szFormat || szBuf == nullptr || szPrefix == nullptr)
			return 0;

		int64_t nCurTime = 0;
		base::time_util::STime sTime;
		if (bGmtTime)
		{
			nCurTime = base::time_util::getGmtTime();
			sTime = base::time_util::getGmtTimeTM(nCurTime);
		}
		else
		{
			nCurTime = base::time_util::getLocalTime();
			sTime = base::time_util::getLocalTimeTM(nCurTime);
		}

		char szTime[30] = { 0 };
		base::function_util::snprintf(szTime, _countof(szTime), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
			sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec, nCurTime % 1000);

		if (nDay != nullptr)
			*nDay = (uint8_t)sTime.nDay;

		size_t nLen = 0;
		if (szPrefix[0] != 0)
			nLen = base::function_util::snprintf(szBuf, nBufSize, "[%s\t] %s ", szPrefix, szTime);
		else
			nLen = base::function_util::snprintf(szBuf, nBufSize, "%s ", szTime);

		nLen += base::function_util::vsnprintf(szBuf + nLen, nBufSize - nLen, szFormat, arg);

		if (nLen >= _LOG_BUF_SIZE - 1)
		{
			// 截断
			base::function_util::strcpy(szBuf + nBufSize - 3, 3, "\r\n");
			nLen = _LOG_BUF_SIZE - 1;
		}
		else
		{
			base::function_util::strcpy(szBuf + nLen, nBufSize - nLen, "\r\n");
			nLen = nLen + 2;
		}

		return (uint32_t)nLen;
	}

	std::string formatLogName(const char* szPath, const char* szSuffix)
	{
		base::time_util::STime sTime = base::time_util::getLocalTimeTM();
		char szBuf[MAX_PATH] = { 0 };
		if (szSuffix[0] == 0)
			base::function_util::snprintf(szBuf, _countof(szBuf), "%s/%s.%4d_%02d_%02d_%02d_%02d_%02d.%d", szPath, base::process_util::getInstanceName(), sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec, base::process_util::getCurrentProcessID());
		else
			base::function_util::snprintf(szBuf, _countof(szBuf), "%s/%s.%4d_%02d_%02d_%02d_%02d_%02d.%d_%s", szPath, base::process_util::getInstanceName(), sTime.nYear, sTime.nMon, sTime.nDay, sTime.nHour, sTime.nMin, sTime.nSec, base::process_util::getCurrentProcessID(), szSuffix);

		return szBuf;
	}

	struct SLogInfo
	{
		char		szSuffix[_LOG_FILE_NAME_SIZE];
		uint32_t	nBufSize;
		uint8_t		bConsole;
		uint8_t		nDay;
		char		szBuf[_LOG_BUF_SIZE];

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
		bool				isAsync() const;
		void				debug(bool bEnable);
		bool				isDebug() const;
		bool				isGmtTime() const;

	private:
		virtual bool		onInit() { return true; }
		virtual void		onDestroy() { }
		virtual bool		onProcess();

		void				flushLog();
		void				tryFlushLog();
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
		std::vector<SLogInfo*>					m_vecLogInfo;
		std::vector<SLogInfo*>					m_vecSwapLogInfo;
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
		this->m_nLastFlushTime = base::time_util::getGmtTime() / 1000;
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

		std::string szTmpPath = szPath;
		if (szTmpPath.empty())
			return false;

		std::string::size_type pos = szTmpPath.find(".");
		if (std::string::npos != pos && pos == 0)
		{
			szTmpPath.replace(pos, 1, "");
			szTmpPath.insert(0, base::process_util::getCurrentWorkPath());
		}

		while (std::string::npos != (pos = szTmpPath.find("\\")))
		{
			szTmpPath.replace(pos, 1, "/");
			pos += 1;
		}

		pos = szTmpPath.find_last_of("/");
		if (pos != szTmpPath.size() - 1)
			szTmpPath.insert(szTmpPath.size(), "/");

		base::function_util::snprintf(this->m_szPath, _countof(this->m_szPath), "%s%s", szTmpPath.c_str(), base::process_util::getInstanceName());

		if (!base::file_util::createRecursionDir(this->m_szPath))
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

		// 有可能还有残留日志需要输出
		this->onProcess();
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
			this->m_vecLogInfo.push_back(pLogInfo);
			this->m_lock.unlock();
		}
		else
		{
			this->saveLog(pLogInfo);
			
			delete pLogInfo;

			this->tryFlushLog();
		}
	}

	bool CLogger::onProcess()
	{
		this->m_lock.lock();
		std::swap(this->m_vecLogInfo, this->m_vecSwapLogInfo);
		this->m_lock.unlock();

		// 这里不用条件变量的原因是因为日志不需要马上输出，用交换队列锁的代价更小。
		if (this->m_vecSwapLogInfo.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			return true;
		}

		for (size_t i = 0; i < this->m_vecSwapLogInfo.size(); ++i)
		{
			SLogInfo* pLogInfo = this->m_vecSwapLogInfo[i];
			if (pLogInfo == nullptr)
				continue;

			if (pLogInfo->nBufSize == 0)
			{
				this->flushLog();
				this->m_nLastFlushTime = base::time_util::getGmtTime();
			}
			else
			{
				this->saveLog(pLogInfo);
			}

			delete pLogInfo;
		}

		this->m_vecSwapLogInfo.clear();

		if (this->m_bAsync)
		{
			this->tryFlushLog();
		}

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

	void CLogger::tryFlushLog()
	{
		int64_t nCurTime = base::time_util::getGmtTime();
		if (nCurTime - this->m_nLastFlushTime >= _FLUSH_LOG_TIME)
		{
			this->flushLog();
			this->m_nLastFlushTime = nCurTime;
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
			base::function_util::snprintf(szFullFileName, _countof(szFullFileName), "%s_%d.log", szFileName.c_str(), pLogFileInfo->nNextFileIndex);
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

// #ifdef _WIN32
// 		fflush(pLogFileInfo->pFile);
// #endif
	}

	bool CLogger::isAsync() const
	{
		return this->m_bAsync;
	}

	CLogger*	g_pLogger;
	FILE*		g_pError;
}

namespace base
{
	namespace log
	{
		bool init(bool bAsync, bool bGmtTime, const char* szPath)
		{
			if (nullptr != g_pLogger)
				return false;

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

		const char* getPath()
		{
			if (g_pLogger == nullptr)
				return nullptr;

			return g_pLogger->getPath();
		}

		bool isAsync()
		{
			if (g_pLogger == nullptr)
				return false;

			return g_pLogger->isAsync();
		}

		void uninit()
		{
			if (g_pError != nullptr)
				fclose(g_pError);

			if (g_pLogger != nullptr)
			{
				g_pLogger->uninit();
				delete g_pLogger;
			}
		}

		void save(const char* szPrefix, bool bConsole, const char* szFormat, ...)
		{
			if (szFormat == nullptr || szPrefix == nullptr)
				return;

			SLogInfo* pLogInfo = new SLogInfo();
			va_list arg;
			va_start(arg, szFormat);
			uint32_t nSize = formatLog(pLogInfo->szBuf, _countof(pLogInfo->szBuf), szPrefix, g_pLogger->isGmtTime(), szFormat, arg, &pLogInfo->nDay);
			va_end(arg);
			if (nSize == 0)
			{
				delete pLogInfo;
				return;
			}

			pLogInfo->nBufSize = nSize;
			pLogInfo->szSuffix[0] = 0;
			pLogInfo->bConsole = bConsole;
			
			g_pLogger->pushLog(pLogInfo);
		}

		void saveEx(const char* szFileName, bool bConsole, const char* szFormat, ...)
		{
			if (szFormat == nullptr || szFileName == nullptr)
				return;

			SLogInfo* pLogInfo = new SLogInfo();
			va_list arg;
			va_start(arg, szFormat);
			uint32_t nSize = formatLog(pLogInfo->szBuf, _countof(pLogInfo->szBuf), "", g_pLogger->isGmtTime(), szFormat, arg, &pLogInfo->nDay);
			va_end(arg);
			if (nSize == 0)
				return;

			pLogInfo->nBufSize = nSize;
			base::function_util::strcpy(pLogInfo->szSuffix, _LOG_FILE_NAME_SIZE, szFileName);
			pLogInfo->bConsole = bConsole;

			g_pLogger->pushLog(pLogInfo);
		}

		void saveError(const char* szFormat, ...)
		{
			if (nullptr == szFormat)
				return;

			static thread_local char szBuf[_LOG_BUF_SIZE] = { 0 };
			uint32_t nSize = (uint32_t)base::getStackInfo(0, 10, szBuf, _countof(szBuf));

			va_list arg;
			va_start(arg, szFormat);
			nSize += formatLog(szBuf + nSize, _countof(szBuf) - nSize, "", g_pLogger->isGmtTime(), szFormat, arg, nullptr);
			va_end(arg);

			fwrite(szBuf, 1, nSize, g_pError);
			std::cout << szBuf;

			fflush(g_pError);
		}

		void flush()
		{
			if (g_pLogger == nullptr)
				return;

			SLogInfo* pLogInfo = new SLogInfo();
			pLogInfo->nBufSize = 0;
			pLogInfo->nDay = 0;
			g_pLogger->pushLog(pLogInfo);
		}

		void debug(bool bEnable)
		{
			if (g_pLogger == nullptr)
				return;

			g_pLogger->debug(bEnable);
		}

		bool isDebug()
		{
			if (g_pLogger == nullptr)
				return false;

			return g_pLogger->isDebug();
		}
	}
}