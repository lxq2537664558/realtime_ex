#pragma once
#include "base_common.h"
#include "string_util.h"

namespace base
{
	namespace log
	{
		__BASE_COMMON_API__ bool		init(bool bAsync, bool bGmtTime, const char* szPath);
		__BASE_COMMON_API__ void		uninit();
		__BASE_COMMON_API__ const char*	getPath();
		__BASE_COMMON_API__ bool		isAsync();
		__BASE_COMMON_API__ void		save(const char* szPrefix, bool bConsole, const char* szFormat, ...);
		__BASE_COMMON_API__ void		saveEx(const char* szFileName, bool bConsole, const char* szFormat, ...);
		__BASE_COMMON_API__ void		saveError(const char* szFormat, ...);
		__BASE_COMMON_API__ void		flush();
		__BASE_COMMON_API__	void		debug(bool bEnable);
		__BASE_COMMON_API__	bool		isDebug();
	}
}

template<class ...Args>
inline void	PrintDebug(const char* szFormat, const Args&... args)
{
	if (!base::log::isDebug())
		return;

	// 先不管性能，等到format函数支持传入buf再改
	std::string szData = base::string_util::format(szFormat, args...);

	base::log::save("DEBUG", true, "%s", szData.c_str());
}

template<class ...Args>
inline void	PrintInfo(const char* szFormat, const Args&... args)
{
	// 先不管性能，等到format函数支持传入buf再改
	std::string szData = base::string_util::format(szFormat, args...);

	base::log::save("INFO", true, "%s", szData.c_str());
}

template<class ...Args>
inline void	PrintWarning(const char* szFormat, const Args&... args)
{
	// 先不管性能，等到format函数支持传入buf再改
	std::string szData = base::string_util::format(szFormat, args...);

	base::log::save("WARN", true, "%s", szData.c_str());
}