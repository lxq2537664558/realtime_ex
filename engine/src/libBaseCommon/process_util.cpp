#include "process_util.h"
#include "file_util.h"
#include "string_util.h"

#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment( lib, "psapi.lib" )
#else
#include <signal.h>
#endif

namespace base
{
	namespace process_util
	{
		static std::string g_szInstanceName;

		const char* getInstanceName()
		{
			return g_szInstanceName.c_str();
		}

		void setInstanceName(const char* szName)
		{
			if (szName == nullptr)
				return;

			std::string szAppName = szName;
#ifdef _WIN32
			size_t nPos = szAppName.find_last_of(L'\\');
#else
			size_t nPos = szAppName.find_last_of(L'/');
#endif
			szAppName = szAppName.substr(nPos + 1, szAppName.size() - nPos);
			nPos = szAppName.find_last_of(L'.');
			if (nPos != std::string::npos)
				szAppName = szAppName.substr(0, nPos);

			g_szInstanceName = szAppName;
		}

		static std::string g_szCurrentWorkPath;

		void setCurrentWorkPath(const char* szPath)
		{
			if (szPath == nullptr)
				return;

			g_szCurrentWorkPath = szPath;
		}

		const char* getCurrentWorkPath()
		{
			return g_szCurrentWorkPath.c_str();
		}

		uint32_t getCurrentProcessID()
		{
#ifdef _WIN32
			return ::GetCurrentProcessId();
#else
			return getpid();
#endif
		}

		bool isProcessExist(uint32_t nProcessID)
		{
#ifdef _WIN32
			HANDLE hHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, nProcessID);
			if (nullptr == hHandle || INVALID_HANDLE_VALUE == hHandle)
				return false;

			::CloseHandle(hHandle);
			return true;
#else
			return kill(nProcessID, 0) != -1;
#endif
		}

		int32_t getProcessorNumber()
		{
			return std::thread::hardware_concurrency();
		}

		uint32_t getPageSize()
		{
			struct SPageSize
			{
				SPageSize()
				{
#ifdef _WIN32
					SYSTEM_INFO systemInfo;
					GetSystemInfo(&systemInfo);
					nPageSize = systemInfo.dwPageSize;
#else
					nPageSize = getpagesize();
#endif
				}

				uint32_t	nPageSize;
			};

			static SPageSize sPageSize;

			return sPageSize.nPageSize;
		}

		size_t getProcStatus(char* szBuf, size_t nBufSize)
		{
			return base::file_util::readFile("/proc/self/status", szBuf, nBufSize);
		}

		size_t getProcStat(char* szBuf, size_t nBufSize)
		{
			return base::file_util::readFile("/proc/self/stat", szBuf, nBufSize);
		}

		size_t getThreadStat(char* szBuf, size_t nBufSize)
		{
			char szFileName[64] = { 0 };
			base::function_util::snprintf(szFileName, sizeof(szFileName), "/proc/self/task/%d/stat", std::this_thread::get_id());

			return base::file_util::readFile(szFileName, szBuf, nBufSize);
		}

		uint32_t getThreadCount()
		{
			char szBuf[4096] = { 0 };
			size_t nSize = getProcStatus(szBuf, _countof(szBuf));
			std::string szStatus(szBuf, nSize);
			size_t nPos = szStatus.find("Threads:");
			if (nPos == std::string::npos)
				return 0;

			uint32_t nCount = 0;
			if (!base::string_util::convert_to_value(szStatus.c_str() + nPos + 8, nCount))
				return 0;

			return nCount;
		}

		int32_t getCPUUsage()
		{
#ifdef _WIN32
			static int32_t nProcessorNumber = -1;
			static LARGE_INTEGER nLastCheckTime;
			static LARGE_INTEGER nLastCheckSystemTime;

			FILETIME curTime;
			FILETIME createTime;
			FILETIME exitTime;
			FILETIME kernelTime;
			FILETIME userTime;

			LARGE_INTEGER nSystemTime;
			LARGE_INTEGER nCurTime;

			int64_t nDeltaSystemTime;
			int64_t nDeltaTime;
			int32_t nCpuUsage = -1;

			::GetSystemTimeAsFileTime(&curTime);

			if (!::GetProcessTimes(::GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime))
			{
				// We don't assert here because in some cases (such as in the Task Manager)
				// we may call this function on a process that has just exited but we have
				// not yet received the notification.
				return -1;
			}

			nSystemTime.LowPart = kernelTime.dwLowDateTime + userTime.dwLowDateTime;
			nSystemTime.HighPart = kernelTime.dwHighDateTime + userTime.dwHighDateTime;

			nCurTime.LowPart = curTime.dwLowDateTime;
			nCurTime.HighPart = curTime.dwHighDateTime;

			if (-1 == nProcessorNumber)
			{
				// First call, just set the last values.
				nProcessorNumber = getProcessorNumber();
				nSystemTime.QuadPart /= nProcessorNumber;
				nLastCheckSystemTime = nSystemTime;
				nLastCheckTime = nCurTime;

				return -1;
			}

			nSystemTime.QuadPart /= nProcessorNumber;

			nDeltaSystemTime = nSystemTime.QuadPart - nLastCheckSystemTime.QuadPart;
			nDeltaTime = nCurTime.QuadPart - nLastCheckTime.QuadPart;

			if (nDeltaTime == 0)
				return -1;

			// We add time_delta / 2 so the result is rounded.
			nCpuUsage = (int32_t)((nDeltaSystemTime * 100 + nDeltaTime / 2) / nDeltaTime);
			nLastCheckSystemTime = nSystemTime;
			nLastCheckTime = nCurTime;

			return nCpuUsage;

#else
			return 0;
#endif
		}

		bool getMemoryUsage(uint64_t& nMem, uint64_t& nVMen)
		{
#ifdef _WIN32
			PROCESS_MEMORY_COUNTERS pmc;
			if (::GetProcessMemoryInfo(::GetCurrentProcess(), &pmc, sizeof(pmc)))
			{
				nMem = pmc.WorkingSetSize;
				nVMen = pmc.PagefileUsage;

				return true;
			}

			return false;
#else
			return false;
#endif
		}

		bool getIOBytes(uint64_t& nReadBytes, uint64_t& nWriteBytes)
		{
#ifdef _WIN32
			IO_COUNTERS io_counter;
			if (::GetProcessIoCounters(::GetCurrentProcess(), &io_counter))
			{
				nReadBytes = io_counter.ReadTransferCount;
				nWriteBytes = io_counter.WriteTransferCount;

				return true;
			}

			return false;
#else
			return false;
#endif
		}
	}
}