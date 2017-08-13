#pragma once

#include <stdio.h>
#include <functional>

#include "base_common.h"
#include "exception_handler.h"
#include "vector2.h"

namespace base
{
	__BASE_COMMON_API__ uint32_t		getLastError();

	__BASE_COMMON_API__ void			sleep(uint32_t milliseconds);

	__BASE_COMMON_API__ uint32_t		hash(const wchar_t* szKey);
	__BASE_COMMON_API__ uint32_t		hash(const char* szKey);

	__BASE_COMMON_API__	uint32_t		crc32(const char* szBuf, uint32_t nBufSize);
	
	__BASE_COMMON_API__ const char*		getInstanceName();
	__BASE_COMMON_API__ void			setInstanceName(const char* szName);
	__BASE_COMMON_API__ void			setCurrentWorkPath(const char* szPath);
	__BASE_COMMON_API__	const char*		getCurrentWorkPath();

	__BASE_COMMON_API__ bool			createDir(const char* szPathName);
	__BASE_COMMON_API__ uint32_t		getCurrentProcessID();
	__BASE_COMMON_API__ bool			isProcessExist(uint32_t nProcessID);

	__BASE_COMMON_API__ int32_t			getProcessorNumber();
	__BASE_COMMON_API__ bool			getMemoryUsage(uint64_t& nMem, uint64_t& nVMen);
	__BASE_COMMON_API__ bool			getIOBytes(uint64_t& nReadBytes, uint64_t& nWriteBytes);
	__BASE_COMMON_API__ int32_t			getCPUUsage();

	__BASE_COMMON_API__ uint16_t		ntoh16(uint16_t x);
	__BASE_COMMON_API__ uint16_t		hton16(uint16_t x);
	__BASE_COMMON_API__ uint32_t		ntoh32(uint32_t x);
	__BASE_COMMON_API__ uint32_t		hton32(uint32_t x);
	__BASE_COMMON_API__ uint64_t		ntoh64(uint64_t x);
	__BASE_COMMON_API__ uint64_t		hton64(uint64_t x);

	template<class T>
	T									limit(T n, T min, T max) { return (n < min) ? min : ((n > max) ? max : n); }

	namespace crt
	{
		__BASE_COMMON_API__ size_t	strnlen(const char* szBuf, size_t nMaxSize);

		__BASE_COMMON_API__ bool	strcpy(char* szDest, size_t nDestSize, const char* szSource);

		__BASE_COMMON_API__ bool	strncpy(char* szDest, size_t nDestSize, const char* szSource, size_t nCount);

		__BASE_COMMON_API__ size_t	snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...);

		__BASE_COMMON_API__ size_t	vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg);

		__BASE_COMMON_API__ bool	itoa(int32_t nValue, char* szBuf, size_t nBufSize);

		__BASE_COMMON_API__ bool	uitoa(uint32_t nValue, char* szBuf, size_t nBufSize);

		__BASE_COMMON_API__ bool	i64toa(int64_t nValue, char* szBuf, size_t nBufSize);
		__BASE_COMMON_API__ bool	ui64toa(uint64_t nValue, char* szBuf, size_t nBufSize);

		__BASE_COMMON_API__ bool	atoi(const char* szBuf, int32_t& nVal);
		__BASE_COMMON_API__ bool	atoui(const char* szBuf, uint32_t& nVal);

		__BASE_COMMON_API__ bool	atoi64(const char* szBuf, int64_t& nVal);
		__BASE_COMMON_API__ bool	atoui64(const char* szBuf, uint64_t& nVal);
	}
}