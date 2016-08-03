#pragma once
#include "base_common.h"

//#define __MEMORY_HOOK__

namespace base
{
	__BASE_COMMON_API__ void*	alloc_hook(size_t nSize, void* pCallAddr);
	__BASE_COMMON_API__ void	dealloc_hook(void* pData);
	__BASE_COMMON_API__ void	beginMemoryLeakChecker(bool bDetail);
	__BASE_COMMON_API__ void	endMemoryLeakChecker(const char* szName);
	__BASE_COMMON_API__ int64_t	getMemorySize();
}