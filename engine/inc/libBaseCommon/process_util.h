#pragma once
#include "base_common.h"

namespace base
{
	namespace process_util
	{
		__BASE_COMMON_API__ const char*		getInstanceName();
		__BASE_COMMON_API__ void			setInstanceName(const char* szName);
		__BASE_COMMON_API__ void			setCurrentWorkPath(const char* szPath);
		__BASE_COMMON_API__	const char*		getCurrentWorkPath();

		__BASE_COMMON_API__ uint32_t		getCurrentProcessID();
		__BASE_COMMON_API__ bool			isProcessExist(uint32_t nProcessID);

		__BASE_COMMON_API__ int32_t			getProcessorNumber();

		__BASE_COMMON_API__ bool			getMemoryUsage(uint64_t& nMem, uint64_t& nVMen);
		__BASE_COMMON_API__ bool			getIOBytes(uint64_t& nReadBytes, uint64_t& nWriteBytes);
		__BASE_COMMON_API__ int32_t			getCPUUsage();

		__BASE_COMMON_API__ uint32_t		getPageSize();


		/// read /proc/self/status
		__BASE_COMMON_API__  size_t			getProcStatus(char* szBuf, size_t nBufSize);

		/// read /proc/self/stat
		__BASE_COMMON_API__ size_t			getProcStat(char* szBuf, size_t nBufSize);

		/// read /proc/self/task/tid/stat
		__BASE_COMMON_API__ size_t			getThreadStat(char* szBuf, size_t nBufSize);

		__BASE_COMMON_API__ uint32_t		getThreadCount();
	}
}