#pragma once
#include "player.h"

#include "libCoreCommon/db_service_invoke_holder.h"

#include <map>

class CGameService;
class CPlayerMgr :
	public core::CDbServiceInvokeHolder
{
public:
	CPlayerMgr(CGameService* pGameService);
	~CPlayerMgr();

	CPlayer*	getPlayer(uint64_t nPlayerID) const;
	CPlayer*	createPlayer(uint64_t nPlayerID, const void* pContext);
	void		destroyPlayer(uint64_t nPlayerID, const std::string& szMsg);

	uint32_t	getPlayerCount() const;

	void		onGateDisconnect(uint32_t nGateServiceID);

private:
	std::map<uint64_t, CPlayer*>	m_mapPlayer;
	CGameService*					m_pGameService;
};