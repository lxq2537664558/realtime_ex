#pragma once

#include "libBaseCommon/base_common.h"
#include "libBaseCommon/noncopyable.h"

#include <vector>


namespace core
{
	class CFixMemoryPool :
		public base::noncopyable
	{
	public:
		CFixMemoryPool(uint32_t nUnitSize, uint32_t nBatchCount);
		~CFixMemoryPool();

		void*	allocate();
		void	deallocate(char* pMemory);

	private:
		std::vector<char*>	m_vecMemory;
		std::vector<char*>	m_vecBatch;
		uint32_t			m_nNextAlloc;
		uint32_t			m_nUnitSize;
		uint32_t			m_nBatchCount;
	};
}