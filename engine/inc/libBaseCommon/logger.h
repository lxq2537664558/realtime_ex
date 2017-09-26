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

	// �Ȳ������ܣ��ȵ�format����֧�ִ���buf�ٸ�
	std::string szData = base::string_util::format(szFormat, args...);

	base::log::save("DEBUG", true, "%s", szData.c_str());
}

template<class ...Args>
inline void	PrintInfo(const char* szFormat, const Args&... args)
{
	// �Ȳ������ܣ��ȵ�format����֧�ִ���buf�ٸ�
	std::string szData = base::string_util::format(szFormat, args...);

	base::log::save("INFO", true, "%s", szData.c_str());
}

template<class ...Args>
inline void	PrintWarning(const char* szFormat, const Args&... args)
{
	// �Ȳ������ܣ��ȵ�format����֧�ִ���buf�ٸ�
	std::string szData = base::string_util::format(szFormat, args...);

	base::log::save("WARN", true, "%s", szData.c_str());
}