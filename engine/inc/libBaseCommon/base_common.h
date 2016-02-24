#pragma once

#include <stdint.h>

#ifndef _WIN32
#include <errno.h>
#include <unistd.h>
#endif

#include <string.h>
#include <limits.h>

#ifdef _WIN32
#pragma warning(error:4244) ///< 把4244号警告信息作为一个错误
#pragma warning(error:4715) ///< 把4715号警告信息作为一个错误
#pragma warning(error:4717) ///< 把4717号警告信息作为一个错误
#endif
#	define INVALID_64BIT UINT64_MAX
#	define INVALID_32BIT UINT32_MAX
#	define INVALID_16BIT UINT16_MAX
#	define INVALID_8BIT  UINT8_MAX

#	define __INVALID_ID 0

#	define OBJID uint64_t

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

#ifdef _WIN32

#else

#	define __max(a, b) ((a)<(b)?(b):(a))
#	define __min(a, b) ((a)<(b)?(a):(b))

#	define ZeroMemory(ptr,size) memset((ptr),0,(size))

#	define Sleep(t) usleep((t)*1000)

#	define stricmp(a,b) strcasecmp( a, b )
#	define strnicmp(a,b, c) strncasecmp( a, b, c )
#	define _strnicmp(a,b,c) strncasecmp( a, b, c )
#	define strncmp(a,b,c) strncmp( a, b, c )
#	define _chdir	chdir
#	define _getcwd	getcwd

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