#pragma once
#include "player.h"

#include "libCoreCommon/db_service_invoke_holder.h"

#include <map>

class CGasService;
class CPlayerMgr :
	public core::CDbServiceInvokeHolder
{
public:
	CPlayerMgr(CGasService* pGasService);
	~CPlayerMgr();

	CPlayer*	getPlayer(uint64_t nPlayerID) const;
	CPlayer*	createPlayer(uint64_t nPlayerID, const std::string& szToken);
	void		destroyPlayer(uint64_t nPlayerID, const std::string& szMsg);

	uint32_t	getPlayerCount() const;

private:
	std::map<uint64_t, CPlayer*>	m_mapPlayer;
	CGasService*					m_pGasService;
};