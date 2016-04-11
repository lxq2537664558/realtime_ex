#include "stdafx.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>
#endif

#include <map>
#include <string>
#include <iostream>
#include <mutex>
#include <thread>

#include "debug_helper.h"
#include "base_function.h"
#include "base_time.h"
#include "spin_mutex.h"

#ifdef _WIN32
#pragma comment( lib, "psapi.lib" )
#endif

namespace base
{
	uint32_t getLastError()
	{
#ifdef _WIN32
		return (uint32_t)GetLastError();
#else
		return (uint32_t)errno;
#endif
	}

	bool isPointInCircularSector(CVector2I center, CVector2I point, float ux, float uy, uint32_t r, uint32_t theta)
	{
		// D = P - C
		double dx = (double)(point.x - center.x);
		double dy = (double)(point.y - center.y);
		// 		///< 这里的距离计算比较特殊，采用r跟dx与dy比较
		// 		if( dx > r && dy > r )
		// 			return false;
		// |D| = (dx^2 + dy^2)^0.5
		double length = sqrt(dx * dx + dy * dy);

		// |D| > r
		if (length > r)
			return false;
		if (length == 0)
			return true;

		// Normalize D
		dx /= length;
		dy /= length;

		double cos_value = dx * ux + dy * uy;
		if (cos_value >= 1.0)
			return true;
		else if (cos_value <= -1.0)
			return false;

		// acos(D dot U) < theta
		double delta = (double)theta / 180.0f*3.1415926f - acos(cos_value);
		if (delta > 0)
			return true;
		if (fabs(delta) < 0.001f)
			return true;

		return false;
	}

	bool utf82Ucs2(const char* szUtf8, wchar_t* szBuf, size_t nBufSize)
	{
		if (nullptr == szUtf8 || nullptr == szBuf)
			return false;

		while (*szUtf8 != 0 && nBufSize > 0)
		{
			if (0x00 == (*szUtf8 & 0x80))
			{
				*szBuf = *szUtf8;
				++szUtf8;
			}
			else if (0xc0 == (*szUtf8 & 0xe0) && 0x80 == (*(szUtf8 + 1) & 0xc0))
			{
				uint16_t nTemp1 = (uint16_t)(*szUtf8 & 0x1f);
				nTemp1 <<= 6;
				nTemp1 |= (unsigned short)(*(szUtf8 + 1) & 0x3f);
				*szBuf = nTemp1;
				szUtf8 += 2;
			}
			else if (0xe0 == (*szUtf8 & 0xf0) && 0x80 == (*(szUtf8 + 1) & 0xc0) && 0x80 == (*(szUtf8 + 2) & 0xc0))
			{
				uint16_t nTemp1 = (uint16_t)(*szUtf8 & 0x0f);
				nTemp1 <<= 12;
				uint16_t nTemp2 = (uint16_t)(*(szUtf8 + 1) & 0x3f);
				nTemp2 <<= 6;
				nTemp1 = nTemp1 | nTemp2 | (uint16_t)(*(szUtf8 + 2) & 0x3f);
				*szBuf = nTemp1;
				szUtf8 += 3;
			}
			else
			{
				*szBuf = 0x22e0;
				return false;
			}
			++szBuf;
			--nBufSize;
		}

		return true;
	}

	bool ucs22Utf8(const wchar_t* szUcs2, char* szBuf, size_t nBufSize)
	{
		if (nullptr == szUcs2 || nullptr == szBuf)
			return false;

		while (*szUcs2 != 0 && nBufSize > 0)
		{
			if (0x0080 > *szUcs2)
			{
				*szBuf = (char)*szUcs2;
				++szBuf;
				--nBufSize;
			}
			else if (0x0800 > *szUcs2)
			{
				*szBuf = ((char)(*szUcs2 >> 6)) | 0xc0;
				*(szBuf + 1) = ((char)(*szUcs2 & 0x003f)) | 0x80;
				szBuf += 2;
				nBufSize -= 2;
			}
			else
			{
				*szBuf = ((char)(*szUcs2 >> 12)) | 0xe0;
				*(szBuf + 1) = ((char)((*szUcs2 & 0x0fc0) >> 6)) | 0x80;
				*(szBuf + 2) = ((char)(*szUcs2 & 0x003f)) | 0x80;
				szBuf += 3;
				nBufSize -= 3;
			}
			++szUcs2;
		}

		return true;
	}

#ifdef WIN32
	bool uft82Ansi(const char* szUft8, char* szBuf, size_t nBufSize)
	{
		if (nullptr == szBuf || nullptr == szUft8)
			return false;

		wchar_t* szUcs2 = new wchar_t[nBufSize];
		if (!utf82Ucs2(szUft8, szUcs2, nBufSize))
		{
			delete[] szUcs2;
			return false;
		}

		bool bRet = ucs22Ansi(szUcs2, szBuf, nBufSize);
		delete[] szUcs2;

		return bRet;
	}

	bool ucs22Ansi(const wchar_t* szUcs2, char* szBuf, size_t nBufSize)
	{
		if (nullptr == szBuf || nullptr == szUcs2)
			return false;

		::WideCharToMultiByte(CP_ACP, 0, szUcs2, -1, szBuf, (int32_t)nBufSize, 0, 0);
		return true;
	}

	bool ansi2Utf8(const char* szAnsi, char* szBuf, size_t nBufSize)
	{
		if (nullptr == szBuf || nullptr == szAnsi)
			return false;

		wchar_t* szUcs2 = new wchar_t[nBufSize];
		if (!ansi2Ucs2(szAnsi, szUcs2, nBufSize))
		{
			delete[] szUcs2;
			return false;
		}

		bool bRet = ucs22Utf8(szUcs2, szBuf, nBufSize);
		delete[] szUcs2;

		return bRet;
	}

	bool ansi2Ucs2(const char* szAnsi, wchar_t* szBuf, size_t nBufSize)
	{
		if (nullptr == szBuf || nullptr == szAnsi)
			return false;

		::MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, szBuf, (int32_t)nBufSize);

		return true;
	}
#endif

	void sleep(uint32_t milliseconds)
	{
#ifdef _WIN32
		::Sleep(milliseconds);
#else
		usleep(milliseconds * 1000);
#endif
	}

	// JS Hash
	uint32_t hash(const wchar_t* szKey)
	{
		if (szKey == nullptr)
			return 0;;

		uint32_t nHash = 1315423911;

		while (*szKey)
		{
			nHash ^= ((nHash << 5) + (*szKey++) + (nHash >> 2));
		}

		return (nHash & 0x7FFFFFFF);
	}

	uint32_t hash(const char* szKey)
	{
		if (szKey == nullptr)
			return 0;;

		uint32_t nHash = 1315423911;

		while (*szKey)
		{
			nHash ^= ((nHash << 5) + (*szKey++) + (nHash >> 2));
		}

		return (nHash & 0x7FFFFFFF);
	}

	bool bresenhamLine(int32_t width, CVector2I begin, CVector2I end, funCheckLine pfCheckLine, void* pContext)
	{
		int32_t dx, dy, x, y, e, i;
		float k;
		if (begin.x > end.x)
		{
			std::swap(begin.x, end.x);
			std::swap(begin.y, end.y);
		}

		int32_t xt = end.x - begin.x;
		int32_t yt = end.y - begin.y;

		dx = std::abs(begin.x - end.x);
		dy = std::abs(begin.y - end.y);
		x = begin.x;
		y = begin.y;

		//处理斜率不存在的情况
		if (xt == 0)
		{
			y = __min(begin.y, end.y);

			for (i = 0; i <= dy; ++i)
			{
				CVector2I point;
				point.x = x;
				point.y = y;

				if (!pfCheckLine(point, pContext))
					return false;

				for (int32_t m = -width / 2; m <= width / 2; ++m)
				{
					point.x = x + m;
					point.y = y;
					if (!pfCheckLine(point, pContext))
						return false;
				}
				++y;
			}
			return true;
		}
		else
			k = (float)yt / (float)xt;

		if (k >= -1 && k <= 1)
		{
			e = 2 * dy - dx;
			for (i = 0; i < dx; ++i)
			{
				CVector2I point;
				point.x = x;
				point.y = y;
				if (!pfCheckLine(point, pContext))
					return false;
				for (int32_t m = -width / 2; m <= width / 2; ++m)
				{
					point.x = x;
					point.y = y + m;
					if (!pfCheckLine(point, pContext))
						return false;
				}
				++x;
				if (e >= 0)
				{
					e += 2 * dy - 2 * dx;
					if (k >= 0 && k <= 1)
						++y;
					else
						--y;
				}
				else
				{
					e += 2 * dy;
				}
			}
		}
		else
		{
			e = 2 * dx - dy;
			for (i = 0; i < dy; ++i)
			{
				CVector2I point;
				point.x = x;
				point.y = y;
				if (!pfCheckLine(point, pContext))
					return false;
				for (int32_t m = -width / 2; m <= width / 2; ++m)
				{
					point.x = x + m;
					point.y = y;
					if (!pfCheckLine(point, pContext))
						return false;
				}
				
				if (k > 1)
					++y;
				else
					--y;

				if (e >= 0)
				{
					e = e + 2 * dx - 2 * dy;
					++x;
				}
				else
					e = e + 2 * dx;
			}
		}

		return true;
	}

	static std::string g_szInstanceName;

	const char* getInstanceName()
	{
		return g_szInstanceName.c_str();
	}

	void setInstanceName(char* szName)
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

	bool createDir(const char* szPathName)
	{
		if (szPathName == nullptr)
			return false;

#ifdef _WIN32

		if (!::CreateDirectoryA(szPathName, nullptr))
		{
			uint32_t nError = getLastError();
			return nError == ERROR_ALREADY_EXISTS;
		}
		return true;
#else
		struct stat statbuf;
		if (lstat(szPathName, &statbuf) != 0)
		{
			if (mkdir(szPathName, 0777) != 0)
			{
				if (lstat(szPathName, &statbuf) == 0)
					return S_ISDIR(statbuf.st_mode);
				else
					return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return S_ISDIR(statbuf.st_mode);
		}
#endif
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
		// 给nProcessID进程发空信号(其实是不发信号,只检测错误)
		return kill(nProcessID, 0) != -1;
#endif
	}

	int32_t getProcessorNumber()
	{
		return std::thread::hardware_concurrency();
	}

	int32_t getCpuUsage()
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


	namespace crt
	{
		size_t strnlen(const char* szBuf, size_t nMaxSize)
		{
			if (szBuf == nullptr)
				return 0;

			if (nMaxSize == 0)
				return ::strlen(szBuf);
			else
				return ::strnlen(szBuf, nMaxSize);
		}

		bool strncpy(char* szDest, size_t nDestSize, const char* szSource, size_t nCount)
		{
#ifdef _WIN32
			return ::strncpy_s(szDest, nDestSize, szSource, nCount) == 0;
#else
			if (nDestSize == 0 || szDest == nullptr || szSource == nullptr)
				return false;

			if (nCount >= nDestSize)
				nCount = nDestSize - 1;

			::strncpy(szDest, szSource, nCount);

			return true;
#endif
		}

		bool strncat(char* szDest, size_t nDestSize, const char* szSource, size_t nCount)
		{
#ifdef _WIN32
			return ::strncat_s(szDest, nDestSize, szSource, nCount) == 0;
#else
			if (nDestSize == 0 || szDest == nullptr || szSource == nullptr)
				return false;

			if (nCount >= nDestSize)
				nCount = nDestSize - 1;

			::strncat(szDest, szSource, nCount);

			return true;
#endif
		}

		size_t snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...)
		{
			if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
				return -1;

			size_t ret = 0;

			va_list arg;
			va_start(arg, szFormat);
#ifdef _WIN32
			ret = ::_vsnprintf_s(szBuf, nBufSize, _TRUNCATE, szFormat, arg);
#else
			ret = ::vsnprintf(szBuf, nBufSize, szFormat, arg);
#endif
			va_end(arg);

			return ret;
		}

		size_t vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg)
		{
			if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
				return -1;

#ifdef _WIN32
			return ::_vsnprintf_s(szBuf, nBufSize, _TRUNCATE, szFormat, arg);
#else
			return ::vsnprintf(szBuf, nBufSize, szFormat, arg);
#endif
		}

		bool itoa(int32_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix)
		{
#ifdef _WIN32
			return (::_itoa_s(nValue, szBuf, nBufSize, nRadix) == 0);
#else
			return true;
#endif
		}

		bool uitoa(uint32_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix)
		{
#ifdef _WIN32
			int64_t n64Value = nValue;
			return (::_ui64toa_s(n64Value, szBuf, nBufSize, nRadix) == 0);
#else
			return true;
#endif
		}

		bool i64toa(int64_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix)
		{
#ifdef _WIN32
			return (::_i64toa_s(nValue, szBuf, nBufSize, nRadix) == 0);
#else
			return true;
#endif
		}

		bool ui64toa(uint64_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix)
		{
#ifdef _WIN32
			return (::_ui64toa_s(nValue, szBuf, nBufSize, nRadix) == 0);
#else
			return true;
#endif
		}

		bool atoi(const char* szBuf, int32_t& nVal)
		{
			nVal = ::strtol(szBuf, NULL, 10);

			return true;
		}

		bool atoui(const char* szBuf, uint32_t& nVal)
		{
#ifdef _WIN32
			errno = 0;
			nVal = ::strtoul(szBuf, NULL, 10);
			return errno == 0;
#else
			return true;
#endif
		}

		bool atoi64(const char* szBuf, int64_t& nVal)
		{
#ifdef _WIN32
			errno = 0;
			nVal = ::_strtoi64(szBuf, NULL, 10);
			return errno == 0;
#else
			return true;
#endif
		}

		bool atoui64(const char* szBuf, uint64_t& nVal)
		{
#ifdef _WIN32
			errno = 0;
			nVal = ::_strtoui64(szBuf, NULL, 10);
			return errno == 0;
#else
			return true;
#endif
		}
	}
}
