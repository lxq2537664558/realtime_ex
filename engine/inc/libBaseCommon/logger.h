#pragma once
#include "base_common.h"
#include "string_util.h"

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

template<class ...Args>
inline void	PrintDebug(const char* szFormat, const Args&... args)
{
	if (!base::isDebugLog())
		return;

	std::string szData = base::string_util::format(szFormat, args...);

	base::saveLog("DEBUG", true, "%s", szData.c_str());
}

template<class ...Args>
inline void	PrintInfo(const char* szFormat, const Args&... args)
{
	std::string szData = base::string_util::format(szFormat, args...);

	base::saveLog("INFO", true, "%s", szData.c_str());
}

template<class ...Args>
inline void	PrintWarning(const char* szFormat, const Args&... args)
{
	std::string szData = base::string_util::format(szFormat, args...);

	base::saveLog("WARN", true, "%s", szData.c_str());
}