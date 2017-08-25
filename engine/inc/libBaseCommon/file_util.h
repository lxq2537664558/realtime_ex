#pragma once

#include "base_common.h"

namespace base
{
	namespace file_util
	{
		__BASE_COMMON_API__ bool	isDirectory(const char* szDir);
		__BASE_COMMON_API__ bool	createRecursionDir(const char* szDir);

		__BASE_COMMON_API__ bool	isFileExist(const char* szFileName);
		__BASE_COMMON_API__ int64_t	getFileSize(const char* szFileName);
		__BASE_COMMON_API__ size_t	readFile(const char* szFileName, char* szBuf, size_t nBufSize);
	}
}