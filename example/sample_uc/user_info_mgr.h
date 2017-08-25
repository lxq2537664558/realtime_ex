#pragma once

#include "user_info.h"

#include <map>

class CUCService;
class CUserInfoMgr
{
public:
	CUserInfoMgr(CUCService* pUCService);
	~CUserInfoMgr();

	CUserInfo*	createUserInfo(uint64_t nPlayerID, uint32_t nGasID);
	void		destroyUserInfo(uint64_t nPlayerID);

	CUserInfo*	getUserInfo(uint64_t nPlayerID) const;

	uint32_t	getUserCount() const;

private:
	std::map<uint64_t, CUserInfo*>	m_mapUserInfo;
	CUCService*						m_pUCService;
};