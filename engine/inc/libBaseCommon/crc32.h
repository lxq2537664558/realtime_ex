#pragma once
#include "base_common.h"

namespace base
{
	namespace crc32
	{
		__BASE_COMMON_API__	uint32_t	encode(const char* szBuf, uint32_t nBufSize);
	}
}