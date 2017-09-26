#pragma once

#include "base_common.h"
#include "defer.h"

namespace base
{
	namespace profiling
	{
		__BASE_COMMON_API__ bool init(bool bEnableProfiling);
		__BASE_COMMON_API__ void enable(bool bEnable);
		__BASE_COMMON_API__ void uninit();
		__BASE_COMMON_API__ void beginByLabel(const char* szLabel, uint32_t nContext);
		__BASE_COMMON_API__ void endByLabel(const char* szLabel, uint32_t nContext);
		__BASE_COMMON_API__ void beginByAddr(const void* pAddr);
		__BASE_COMMON_API__ void endByAddr(const void* pAddr);
		__BASE_COMMON_API__ void update(int64_t nTotalTime);
	}
}

#define __PROFILING_OPEN

#ifndef __PROFILING_OPEN
#	define PROFILING_BEGIN(Label)
#	define PROFILING_END(Label)
#	define PROFILING_GUARD(Label)

#	define PROFILING_BEGIN_EX(Label, Context)
#	define PROFILING_END_EX(Label, Context)
#	define PROFILING_GUARD_EX(Label, Context)
#else
#	define PROFILING_BEGIN(Label)					base::profiling::beginByLabel(#Label, -1);
#	define PROFILING_END(Label)						base::profiling::endByLabel(#Label, -1);
#	define PROFILING_GUARD(Label)					base::profiling::beginByLabel(#Label, -1); defer([](){ base::profiling::endByLabel(#Label, -1); });

#	define PROFILING_BEGIN_EX(Label, Context)		base::profiling::beginByLabel(#Label, Context);
#	define PROFILING_END_EX(Label, Context)			base::profiling::endByLabel(#Label, Context);
#	define PROFILING_GUARD_EX(Label, Context)		base::profiling::beginByLabel(#Label, Context); defer([](){ base::profiling::endByLabel(#Label, Context); });
#endif