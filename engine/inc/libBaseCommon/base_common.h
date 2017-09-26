#pragma once

#include <stdint.h>

#ifndef _WIN32
#include <errno.h>
#include <unistd.h>
#endif

#include <string.h>
#include <limits.h>
#include <memory>

#ifdef _WIN32
#pragma warning(error:4244)
#pragma warning(error:4715)
#pragma warning(error:4717)
#endif
#	define INVALID_64BIT UINT64_MAX
#	define INVALID_32BIT UINT32_MAX
#	define INVALID_16BIT UINT16_MAX
#	define INVALID_8BIT  UINT8_MAX

#	define __INVALID_ID 0

#ifndef INFINITE
#	define INFINITE 0xFFFFFFFF
#endif

typedef unsigned long		_ulong;

#ifdef _WIN32

#	define INT64FMT		"%I64d"
#	define UINT64FMT	"%I64u"
#	define XINT64FMT	"%I64x"
#	define OBJIDFMT		"%I64u"

#else

#	define INT64FMT		"%lld"
#	define UINT64FMT	"%llu"
#	define XINT64FMT	"%llx"
#	define OBJIDFMT		"%llu"

#define MAX_PATH PATH_MAX
#endif

#ifndef _countof
#	define _countof(elem) sizeof(elem)/sizeof(elem[0])
#endif

#ifndef _TRUNCATE
#	define _TRUNCATE (size_t)-1
#endif

#ifndef _WIN32
#	define stricmp(a,b) strcasecmp(a, b)
#	define strnicmp(a,b, c) strncasecmp(a, b, c)
#	define _strnicmp(a,b,c) strncasecmp(a, b, c)
#	define strncmp(a,b,c) strncmp(a, b, c)
#	define _chdir	chdir
#	define _getcwd	getcwd

namespace std
{
	// 这里用std名字空间是迫不得已，因为make_unique是在c++14中定义，工程是以c++11方式编译
#if __cplusplus <= 201103L
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
#endif
}

#endif


#ifdef _WIN32

#	ifdef __BUILD_BASE_COMMON_DLL__
#		define __BASE_COMMON_API__ __declspec(dllexport)
#	else
#		define __BASE_COMMON_API__ __declspec(dllimport)
#	endif

#else

#	define __BASE_COMMON_API__

#endif