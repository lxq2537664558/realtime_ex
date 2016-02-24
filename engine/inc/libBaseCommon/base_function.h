#pragma once

#include <stdio.h>
#include <functional>

#include "base_common.h"
#include "exception_handler.h"
#include "vector2.h"

namespace base
{
	typedef std::function<bool(const CVector2I& point, void* pContext)>	funCheckLine;

	__BASE_COMMON_API__ bool			bresenhamLine(int32_t width, CVector2I begin, CVector2I end, funCheckLine pfCheckLine, void* pContext);
	__BASE_COMMON_API__ bool			isPointInCircularSector(CVector2I center, CVector2I point, float ux, float uy, uint32_t r, uint32_t theta);

	__BASE_COMMON_API__ uint32_t		getLastError();

	__BASE_COMMON_API__ bool			utf82Ucs2(const char* szUtf8, wchar_t* szBuf, size_t nBufSize);
	__BASE_COMMON_API__ bool			ucs22Utf8(const wchar_t* szUcs2, char* szBuf, size_t nBufSize);

#ifdef WIN32
	__BASE_COMMON_API__ bool			uft82Ansi(const char* szUft8, char* szBuf, size_t nBufSize);
	__BASE_COMMON_API__ bool			ucs22Ansi(const wchar_t* szUcs2, char* szBuf, size_t nBufSize);
	__BASE_COMMON_API__ bool			ansi2Utf8(const char* szAnsi, char* szBuf, size_t nBufSize);
	__BASE_COMMON_API__ bool			ansi2Ucs2(const char* szAnsi, wchar_t* szBuf, size_t nBufSize);
#endif

	__BASE_COMMON_API__ void			sleep(uint32_t milliseconds);
	__BASE_COMMON_API__ uint32_t		hash(const wchar_t* szKey);
	__BASE_COMMON_API__ uint32_t		hash(const char* szKey);

	// 这两个函数是有全局变量的
	__BASE_COMMON_API__ const char*		getInstanceName();
	__BASE_COMMON_API__ void			setInstanceName(char* szName);
	__BASE_COMMON_API__ void			setCurrentWorkPath(const char* szPath);
	__BASE_COMMON_API__	const char*		getCurrentWorkPath();

	__BASE_COMMON_API__ bool			createDir(const char* szPathName);
	__BASE_COMMON_API__ uint32_t		getCurrentProcessID();
	__BASE_COMMON_API__ bool			isProcessExist(uint32_t nProcessID);

	__BASE_COMMON_API__ int32_t			getProcessorNumber();
	__BASE_COMMON_API__ bool			getMemoryUsage(uint64_t& nMem, uint64_t& nVMen);
	__BASE_COMMON_API__ bool			getIOBytes(uint64_t& nReadBytes, uint64_t& nWriteBytes);
	__BASE_COMMON_API__ int32_t			getCpuUsage();

	template<class T>
	T									limit(T n, T min, T max) { return (n < min) ? min : ((n > max) ? max : n); }


	// crt中可能会出现内存溢出的函数这里全部重新定义，项目中禁止调用c runtime 中的类似函数，全部调用这边对应的函数
	namespace crt
	{
		__BASE_COMMON_API__ size_t	strnlen(const char* szBuf, size_t nMaxSize);

		__BASE_COMMON_API__ bool	strncpy(char* szDest, size_t nDestSize, const char* szSource, size_t nCount);

		__BASE_COMMON_API__ bool	strncat(char* szDest, size_t nDestSize, const char* szSource, size_t nCount);

		__BASE_COMMON_API__ size_t	snprintf(char* szBuf, size_t nBufSize, const char* szFormat, ...);

		__BASE_COMMON_API__ size_t	vsnprintf(char* szBuf, size_t nBufSize, const char* szFormat, va_list arg);

		__BASE_COMMON_API__ bool	itoa(int32_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix);

		__BASE_COMMON_API__ bool	uitoa(uint32_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix);

		__BASE_COMMON_API__ bool	i64toa(int64_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix);
		__BASE_COMMON_API__ bool	ui64toa(uint64_t nValue, char* szBuf, size_t nBufSize, uint32_t nRadix);

		__BASE_COMMON_API__ bool	atoi(const char* szBuf, int32_t& nVal);
		__BASE_COMMON_API__ bool	atoui(const char* szBuf, uint32_t& nVal);

		__BASE_COMMON_API__ bool	atoi64(const char* szBuf, int64_t& nVal);
		__BASE_COMMON_API__ bool	atoui64(const char* szBuf, uint64_t& nVal);
	}
}