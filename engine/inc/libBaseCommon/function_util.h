#pragma once

#include "base_common.h"

#include <stdarg.h>

namespace base
{
	__BASE_COMMON_API__ uint32_t		getLastError();

	namespace function_util
	{
		__BASE_COMMON_API__ uint32_t	hash(const wchar_t* szKey);
		__BASE_COMMON_API__ uint32_t	hash(const char* szKey);

		__BASE_COMMON_API__ uint16_t	ntoh16(uint16_t x);
		__BASE_COMMON_API__ uint16_t	hton16(uint16_t x);
		__BASE_COMMON_API__ uint32_t	ntoh32(uint32_t x);
		__BASE_COMMON_API__ uint32_t	hton32(uint32_t x);
		__BASE_COMMON_API__ uint64_t	ntoh64(uint64_t x);
		__BASE_COMMON_API__ uint64_t	hton64(uint64_t x);

		template<class T>
		T								limit(T n, T min, T max) { return (n < min) ? min : ((n > max) ? max : n); }

		__BASE_COMMON_API__ size_t		strnlen(const char* szBuf, size_t nMaxSize);

		__BASE_COMMON_API__ bool		strcpy(char* szDest, size_t nDestSize, const char* szSource);

		__BASE_COMMON_API__ bool		strncpy(char* szDest, size_t nDestSize, const char* szSource, size_t nCount);

		__BASE_COMMON_API__ size_t		snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...);

		__BASE_COMMON_API__ size_t		vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg);
	}
}