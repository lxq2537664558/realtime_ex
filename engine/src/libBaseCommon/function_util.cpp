#include "stdafx.h"

#ifndef _WIN32
#include <stdarg.h>
#endif

#include <string>
#include <sstream>

#include "debug_helper.h"
#include "function_util.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4996)
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

	namespace function_util
	{
		// JS Hash
		uint32_t hash(const wchar_t* szKey)
		{
			if (szKey == nullptr)
				return 0;

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
				return 0;

			uint32_t nHash = 1315423911;

			while (*szKey)
			{
				nHash ^= ((nHash << 5) + (*szKey++) + (nHash >> 2));
			}

			return (nHash & 0x7FFFFFFF);
		}

		uint16_t ntoh16(uint16_t x)
		{
			return ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
		}

		uint16_t hton16(uint16_t x)
		{
			return ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
		}

		uint32_t hton32(uint32_t x)
		{
			return (x << 24) |
				(x & 0x0000ff00) << 8 |
				(x & 0x00ff0000) >> 8 |
				x >> 24;
		}

		uint32_t ntoh32(uint32_t x)
		{
			return (x << 24) |
				(x & 0x0000ff00) << 8 |
				(x & 0x00ff0000) >> 8 |
				x >> 24;
		}

		uint64_t ntoh64(uint64_t x)
		{
			return (x >> 56) |
				((x << 40) & 0x00ff000000000000LL) |
				((x << 24) & 0x0000ff0000000000LL) |
				((x << 8) & 0x000000ff00000000LL) |
				((x >> 8) & 0x00000000ff000000LL) |
				((x >> 24) & 0x0000000000ff0000LL) |
				((x >> 40) & 0x000000000000ff00LL) |
				(x << 56);
		}

		uint64_t hton64(uint64_t x)
		{
			return (x >> 56) |
				((x << 40) & 0x00ff000000000000LL) |
				((x << 24) & 0x0000ff0000000000LL) |
				((x << 8) & 0x000000ff00000000LL) |
				((x >> 8) & 0x00000000ff000000LL) |
				((x >> 24) & 0x0000000000ff0000LL) |
				((x >> 40) & 0x000000000000ff00LL) |
				(x << 56);
		}

		size_t strnlen(const char* szBuf, size_t nMaxSize)
		{
			if (szBuf == nullptr)
				return 0;

			if (nMaxSize == 0)
				return ::strlen(szBuf);
			else
				return ::strnlen(szBuf, nMaxSize);
		}

		// 如果目标字符串足以放下源字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
		bool strcpy(char* szDest, size_t nDestSize, const char* szSource)
		{
			return strncpy(szDest, nDestSize, szSource, _TRUNCATE);
		}

		// 如果目标字符串足以放下源字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
		bool strncpy(char* szDest, size_t nDestSize, const char* szSource, size_t nCount)
		{
			if (nDestSize == 0 || szDest == nullptr || szSource == nullptr)
				return false;

			if (nCount >= nDestSize)
				nCount = nDestSize - 1;

			// 这里不想用windows下的s系列函数，因为这样会导致跟linux的语义不一致（如果内存不够不是尽量拷贝）
			::strncpy(szDest, szSource, nCount);

			szDest[nCount] = 0;

			return true;
		}
		// 如果目标字符串足以放下格式化后字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
		size_t snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...)
		{
			if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
				return -1;

			va_list arg;
			va_start(arg, szFormat);
			size_t ret = vsnprintf(szBuf, nBufSize, szFormat, arg);
			va_end(arg);

			return ret;
		}
		// 如果目标字符串足以放下格式化后字符串的内容（包括\0），那么就直接拷贝，不然就会截断以保证目标字符串一定是以\0结尾
		size_t vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg)
		{
			if (nBufSize == 0 || szBuf == nullptr || szFormat == nullptr)
				return -1;

#ifdef _WIN32
			// 这里用旧的函数是为了跟linux相同语义
			int32_t nRet = ::_vsnprintf(szBuf, nBufSize, szFormat, arg);
			if( nRet == nBufSize)
			{
				szBuf[nBufSize - 1] = 0;
				--nRet;
			}
			return nRet;
#else
			return ::vsnprintf(szBuf, nBufSize, szFormat, arg);
#endif
		}
	}
}

#ifdef _WIN32
#pragma warning(pop)
#endif
