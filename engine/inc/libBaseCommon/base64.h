#pragma once
#include "base_common.h"

namespace base
{
	namespace base64
	{
		__BASE_COMMON_API__ int32_t encode(const char* szData, uint32_t nLen, char* szBuf, uint32_t nBufSize);
		__BASE_COMMON_API__ int32_t decode(const char* szData, uint32_t nLen, char* szBuf, uint32_t nBufSize);
	}
}