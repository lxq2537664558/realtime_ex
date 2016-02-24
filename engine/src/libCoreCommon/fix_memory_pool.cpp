#include "stdafx.h"
#include "fix_memory_pool.h"
#include "libBaseCommon/debug_helper.h"

namespace core
{

	CFixMemoryPool::CFixMemoryPool(uint32_t nUnitSize, uint32_t nBatchCount)
		: m_nNextAlloc(0)
		, m_nUnitSize(nUnitSize)
		, m_nBatchCount(nBatchCount)
	{
	}

	CFixMemoryPool::~CFixMemoryPool()
	{
		for (size_t i = 0; i < this->m_vecBatch.size(); ++i)
		{
			char* pBatch = this->m_vecBatch[i];
			SAFE_DELETE_ARRAY(pBatch);
		}
	}

	void* CFixMemoryPool::allocate()
	{
		if (this->m_nNextAlloc >= this->m_vecMemory.size())
		{
			size_t nSize = this->m_nUnitSize * this->m_nBatchCount;

			char* pBatch = new char[nSize];
			this->m_vecBatch.push_back(pBatch);

			nSize = this->m_vecMemory.size() + this->m_nBatchCount;
			this->m_vecMemory.resize(nSize);

			uint32_t nIndex = this->m_nNextAlloc;
			for (uint32_t i = 0; i < this->m_nBatchCount; ++i)
			{
				this->m_vecMemory[nIndex++] = &pBatch[this->m_nUnitSize * i];
			}
		}

		return this->m_vecMemory[this->m_nNextAlloc++];
	}

	void CFixMemoryPool::deallocate(char* pMemory)
	{
		if (pMemory != nullptr && this->m_nNextAlloc > 0)
			this->m_vecMemory[--this->m_nNextAlloc] = pMemory;
	}

}