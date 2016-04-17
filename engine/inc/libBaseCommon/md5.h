#pragma once
#include "base_common.h"

namespace base
{
	__BASE_COMMON_API__ void md5(const char* szSrc, char* szOut, size_t nOutLen);
	__BASE_COMMON_API__ bool hexEncode(const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen);
	__BASE_COMMON_API__ bool hexDecode(const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen);
	__BASE_COMMON_API__ bool base64Encode(const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen);
	__BASE_COMMON_API__ bool base64Decode(const char* szIn, size_t nInLen, char* szOut, size_t& nOutLen);
}

#define MD5_RESULT_LEN 16