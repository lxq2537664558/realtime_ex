#include "user_info_mgr.h"

#include "libBaseCommon/debug_helper.h"

CUserInfoMgr::CUserInfoMgr(CUCService* pUCService)
	: m_pUCService(pUCService)
{

}

CUserInfoMgr::~CUserInfoMgr()
{

}

CUserInfo* CUserInfoMgr::createUserInfo(uint64_t nPlayerID, uint32_t nGasID)
{
	if (this->getUserInfo(nPlayerID) != nullptr)
		return nullptr;

	CUserInfo* pUserInfo = new CUserInfo(this->m_pUCService, nPlayerID, nGasID);

	this->m_mapUserInfo[nPlayerID] = pUserInfo;

	return pUserInfo;
}

CUserInfo* CUserInfoMgr::getUserInfo(uint64_t nPlayerID) const
{
	auto iter = this->m_mapUserInfo.find(nPlayerID);
	if (iter == this->m_mapUserInfo.end())
		return nullptr;

	return iter->second;
}

uint32_t CUserInfoMgr::getUserCount() const
{
	return (uint32_t)this->m_mapUserInfo.size();
}

void CUserInfoMgr::destroyUserInfo(uint64_t nPlayerID)
{
	auto iter = this->m_mapUserInfo.find(nPlayerID);
	if (iter == this->m_mapUserInfo.end())
		return;

	CUserInfo* pUserInfo = iter->second;
	this->m_mapUserInfo.erase(nPlayerID);

	SAFE_DELETE(pUserInfo);
}