#pragma once

#include "base_common.h"
#include "defer.h"

namespace base
{
	__BASE_COMMON_API__ bool initProfiling(bool bProfiling);
	__BASE_COMMON_API__ void enableProfiling(bool bProfiling);
	__BASE_COMMON_API__ void uninitProfiling();
	__BASE_COMMON_API__ void profilingBeginByLabel(const char* szLabel, uint32_t nContext);
	__BASE_COMMON_API__ void profilingEndByLabel(const char* szLabel, uint32_t nContext);
	__BASE_COMMON_API__ void profilingBeginByAddr(const void* pAddr);
	__BASE_COMMON_API__ void profilingEndByAddr(const void* pAddr);
	__BASE_COMMON_API__ void profiling(int64_t nTotalTime);
}

//#define __PROFILING_OPEN

#ifndef __PROFILING_OPEN
#	define PROFILING_BEGIN(Label)
#	define PROFILING_END(Label)
#	define PROFILING_GUARD(Label)

#	define PROFILING_BEGIN_EX(Label, Context)
#	define PROFILING_END_EX(Label, Context)
#	define PROFILING_GUARD_EX(Label, Context)
#else
#	define PROFILING_BEGIN(Label)					base::profilingBeginByLabel(#Label, -1);
#	define PROFILING_END(Label)						base::profilingEndByLabel(#Label, -1);
#	define PROFILING_GUARD(Label)					base::profilingBeginByLabel(#Label, -1); Defer(base::profilingEndByLabel(#Label, -1););

#	define PROFILING_BEGIN_EX(Label, Context)		base::profilingBeginByLabel(#Label, Context);
#	define PROFILING_END_EX(Label, Context)			base::profilingEndByLabel(#Label, Context);
#	define PROFILING_GUARD_EX(Label, Context)		base::profilingBeginByLabel(#Label, Context); Defer(base::profilingEndByLabel(#Label, Context););
#endif