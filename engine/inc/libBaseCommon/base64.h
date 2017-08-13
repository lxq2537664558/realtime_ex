#pragma once
#include "base_common.h"

namespace base
{
	__BASE_COMMON_API__ int32_t base64_encode(const char* szData, uint32_t nLen, char* szBuf, uint32_t nBufSize);
	__BASE_COMMON_API__ int32_t base64_decode(const char* szData, uint32_t nLen, char* szBuf, uint32_t nBufSize);
}