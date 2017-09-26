#include "online_count_mgr.h"
#include "libBaseCommon/logger.h"
#include "../common/common.h"

COnlineCountMgr::COnlineCountMgr()
{

}

COnlineCountMgr::~COnlineCountMgr()
{

}

uint32_t COnlineCountMgr::getSuitableGasID() const
{
	uint32_t nMinCount = UINT32_MAX;
	uint32_t nSuitableGasID = 0;
	for (auto iter = this->m_mapGasOnline.begin(); iter != this->m_mapGasOnline.end(); ++iter)
	{
		if (iter->second < nMinCount)
		{
			nMinCount = iter->second;
			nSuitableGasID = iter->first;
		}
	}

	return nSuitableGasID;
}

uint32_t COnlineCountMgr::getSuitableGateID() const
{
	uint32_t nMinCount = UINT32_MAX;
	uint32_t nSuitableGateID = 0;
	for (auto iter = this->m_mapGateOnline.begin(); iter != this->m_mapGateOnline.end(); ++iter)
	{
		if (iter->second < nMinCount)
		{
			nMinCount = iter->second;
			nSuitableGateID = iter->first;
		}
	}

	return nSuitableGateID;
}

uint32_t COnlineCountMgr::getSuitableUCID() const
{
	uint32_t nMinCount = UINT32_MAX;
	uint32_t nSuitableUCID = 0;
	for (auto iter = this->m_mapUCActive.begin(); iter != this->m_mapUCActive.end(); ++iter)
	{
		if (iter->second < nMinCount)
		{
			nMinCount = iter->second;
			nSuitableUCID = iter->first;
		}
	}

	return nSuitableUCID;
}

void COnlineCountMgr::setGasOnlineCount(uint32_t nGasID, uint32_t nCount)
{
	this->m_mapGasOnline[nGasID] = nCount;
}

void COnlineCountMgr::setGateOnlineCount(uint32_t nGateID, uint32_t nCount)
{
	this->m_mapGateOnline[nGateID] = nCount;
}

void COnlineCountMgr::setGateAddr(uint32_t nGateID, const std::string& szAddr)
{
	this->m_mapGateAddr[nGateID] = szAddr;
}

const std::string& COnlineCountMgr::getGateAddr(uint32_t nGateID) const
{
	auto iter = this->m_mapGateAddr.find(nGateID);
	if (iter == this->m_mapGateAddr.end())
	{
		static std::string s_Addr;
		return s_Addr;
	}

	return iter->second;
}

void COnlineCountMgr::setUCActiveCount(uint32_t nUCID, uint32_t nCount)
{
	this->m_mapUCActive[nUCID] = nCount;
}