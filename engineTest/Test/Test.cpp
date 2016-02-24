// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdint.h>
#include <functional>
#include <iostream>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <list>
using namespace std;

// struct SRandData
// {
// 	uint32_t	nKey;
// 	uint32_t	nRandIndex;
// };
// 
// class CRandBase
// {
// public:
// 	CRandBase(uint32_t nChunkSize) : m_nChunkSize(nChunkSize) { }
// 	~CRandBase();
// 
// 	void updateData(SRandData* pRandData);
// 
// private:
// 	struct SChunkInfo
// 	{
// 		std::list<SRandData*>	listData;
// 		uint32_t				nMinKey;
// 		uint32_t				nMaxKey;
// 	};
// 
// 	std::vector<SChunkInfo*>	m_vecChunkInfo;
// 	uint32_t					m_nChunkSize;
// };
// 
// void CRandBase::updateData(SRandData* pRandData)
// {
// 	if (nullptr == pRandData)
// 		return;
// 
// 	auto iter = std::lower_bound(this->m_vecChunkInfo.begin(), this->m_vecChunkInfo.end(), pRandData, [](const SRandData* lhs, const SRandData* rhs) -> bool{ return lhs->nKey > rhs->nKey; });
// 	if (iter == this->m_vecChunkInfo.end())
// 	{
// 		if (this->m_vecChunkInfo.empty())
// 		{
// 			SChunkInfo* pChunkInfo = new SChunkInfo();
// 			pChunkInfo->listData.push_back(pRandData);
// 			pChunkInfo->nMinKey = pRandData->nKey;
// 			pChunkInfo->nMaxKey = pRandData->nKey + 1;
// 			this->m_vecChunkInfo.push_back(pChunkInfo);
// 			pRandData->nRandIndex = 0;
// 		}
// 		else
// 		{
// 
// 		}
// 	}
// }

int main()
{
	uint32_t a = -664642962;
	return 0;
}