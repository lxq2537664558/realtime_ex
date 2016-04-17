#pragma once

#include "base_common.h"

#include <functional>

namespace base
{
	typedef std::function<void(void)> funExitHandler;
	__BASE_COMMON_API__ size_t	getStackInfo(uint32_t nBegin, uint32_t nEnd, char* szInfo, size_t nMaxSize);
	__BASE_COMMON_API__ size_t	getFunctionInfo(const void* pAddr, char* szInfo, size_t nMaxSize);
	__BASE_COMMON_API__ void	initProcessExceptionHander();
	__BASE_COMMON_API__ void	initThreadExceptionHander();
	__BASE_COMMON_API__ void	uninitProcessExceptionHander();
}