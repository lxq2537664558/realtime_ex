#pragma once
#include "base_common.h"

namespace base
{
	__BASE_COMMON_API__ bool	initLog(bool bAsync, bool bGmtTime, const char* szPath);
	__BASE_COMMON_API__ void	uninitLog();
	__BASE_COMMON_API__ void	saveLog(const char* szPrefix, bool bConsole, const char* szFormat, ...);
	__BASE_COMMON_API__ void	saveLogEx(const char* szFileName, bool bConsole, const char* szFormat, ...);
	__BASE_COMMON_API__ void	saveErrorLog(const char* szFormat, ...);
	__BASE_COMMON_API__ void	flushLog();
	__BASE_COMMON_API__	void	debugLog(bool bEnable);
	__BASE_COMMON_API__	bool	isDebugLog();
}

#ifdef _WIN32

#define PrintDebug(szFormat, ...)	\
	do\
	{\
			if (base::isDebugLog())\
			base::saveLog("DEBUG", true, szFormat, __VA_ARGS__);\
	} while(0)


#define PrintInfo(szFormat, ...)		base::saveLog("INFO", true, szFormat, __VA_ARGS__)
#define PrintWarning(szFormat, ...)		base::saveLog("WARN", true, szFormat, __VA_ARGS__)

#else

#define PrintDebug(szFormat, args...)	\
	do\
	{\
			if (base::isDebugLog())\
			base::saveLog("DEBUG", true, szFormat, ##args);\
	} while(0)

#define PrintInfo(szFormat, args...)	base::saveLog("INFO", true, szFormat, ##args)
#define PrintWarning(szFormat, args...)	base::saveLog("WARN", true, szFormat, ##args)

#endif