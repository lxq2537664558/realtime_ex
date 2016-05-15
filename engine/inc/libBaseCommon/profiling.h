#pragma once

#include "base_common.h"
#include "defer.h"

namespace base
{
	__BASE_COMMON_API__ bool initProfiling(bool bProfiling);
	__BASE_COMMON_API__ void enableProfiling(bool bProfiling);
	__BASE_COMMON_API__ void uninitProfiling();
	__BASE_COMMON_API__ void profilingBeginByLabel(const char* szLabel);
	__BASE_COMMON_API__ void profilingEndByLabel(const char* szLabel);
	__BASE_COMMON_API__ void profilingBeginByAddr(const void* pAddr);
	__BASE_COMMON_API__ void profilingEndByAddr(const void* pAddr);
	__BASE_COMMON_API__ void profiling(int64_t nTotalTime);
}

#define __PROFILING_OPEN

#ifndef __PROFILING_OPEN
#	define PROFILING_BEGIN(Label)
#	define PROFILING_END(Label)
#	define PROFILING_GUARD(Label)
#else
#	define PROFILING_BEGIN(Label)					base::profilingBeginByLabel(#Label);
#	define PROFILING_END(Label)						base::profilingEndByLabel(#Label);
#	define PROFILING_GUARD(Label)					base::profilingBeginByLabel(#Label); Defer(base::profilingEndByLabel(#Label););
#endif