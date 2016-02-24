#pragma once
#include "base_common.h"

namespace base
{
	__BASE_COMMON_API__ bool	initLog(bool bAsync);
	__BASE_COMMON_API__ void	uninitLog();
	__BASE_COMMON_API__ void	saveLog(const char* szSection, bool bConsole, const char* szFormat, ...);
	__BASE_COMMON_API__ void	saveLogEx(const char* szFileName, bool bConsole, const char* szFormat, ...);
	__BASE_COMMON_API__ void	saveErrorLog(const char* szFormat, ...);
	__BASE_COMMON_API__ void	flushLog();
}

#ifdef _WIN32

#ifdef _DEBUG
#define PrintDebug(szFormat, ...)		base::saveLog("DEBUG   ", true, szFormat, __VA_ARGS__)
#else
#define PrintDebug(szFormat, ...)
#endif

#define PrintInfo(szFormat, ...)		base::saveLog("INFO    ", true, szFormat, __VA_ARGS__)
#define PrintWarning(szFormat, ...)		base::saveLog("WARNING ", true, szFormat, __VA_ARGS__)

#else

#ifdef _DEBUG
#define PrintDebug(szFormat, args...)	base::saveLog("DEBUG   ", true, szFormat, ##args)
#else
#define PrintDebug(szFormat, args...)
#endif

#define PrintInfo(szFormat, args...)	base::saveLog("DEBUG   ", true, szFormat, ##args)
#define PrintWarning(szFormat, args...)	base::saveLog("WARNING ", true, szFormat, ##args)

#endif