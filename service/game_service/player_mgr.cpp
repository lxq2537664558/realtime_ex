#include "player_mgr.h"
#include "game_service.h"
#include "../common/common.h"
#include "server_proto_src/s2u_player_leave_notify.pb.h"

CPlayerMgr::CPlayerMgr(CGameService* pGameService)
	: CDbServiceInvokeHolder(pGameService)
	, m_pGameService(pGameService)
{

}

CPlayerMgr::~CPlayerMgr()
{

}

CPlayer* CPlayerMgr::getPlayer(uint64_t nPlayerID) const
{
	auto iter = this->m_mapPlayer.find(nPlayerID);
	if (iter == this->m_mapPlayer.end())
		return nullptr;

	return iter->second;
}

CPlayer* CPlayerMgr::createPlayer(uint64_t nPlayerID, const void* pContext)
{
	if (nullptr != this->getPlayer(nPlayerID))
		return nullptr;

	CActorBase* pActorBase = this->m_pGameService->createActor("CPlayer", nPlayerID, pContext);
	if (nullptr == pActorBase)
		return nullptr;

	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pActorBase);
	if (nullptr == pPlayer)
	{
		this->m_pGameService->destroyActor(pActorBase);
		return nullptr;
	}
	
	this->m_mapPlayer[nPlayerID] = pPlayer;

	PrintInfo("CPlayerMgr::createPlayer player_id: {}", nPlayerID);

	pPlayer->onLoadData();

	uint32_t nDBID = _GET_PLAYER_DB_ID(nPlayerID);
	uint32_t nDbServiceID = nDBID + _GAME_DB_SERVICE_DELTA;
	uint32_t nErrorCode = this->sync_nop(nDbServiceID, (uint32_t)nPlayerID);
	
	pPlayer = this->getPlayer(nPlayerID);
	if (nullptr == pPlayer)
	{
		PrintInfo("load player data nop nullptr == pPlayer player_id: {}", nPlayerID);
		return nullptr;
	}

	if (nErrorCode != eRRT_OK)
	{
		PrintInfo("load player data nop nErrorCode != eRRT_OK player_id: {}", nPlayerID);
		this->destroyPlayer(nPlayerID ,"nop error");
		return nullptr;
	}

	if (pPlayer->hasModuleLoadDataError())
	{
		PrintInfo("load player data error pPlayer->hasModuleLoadDataError() player_id: {}", nPlayerID);
		this->destroyPlayer(nPlayerID, "load player data error");
		return nullptr;
	}

	return pPlayer;
}

void CPlayerMgr::destroyPlayer(uint64_t nPlayerID, const std::string& szMsg)
{
	auto iter = this->m_mapPlayer.find(nPlayerID);
	if (iter == this->m_mapPlayer.end())
		return;

	CPlayer* pPlayer = iter->second;
	this->m_mapPlayer.erase(iter);

	this->m_pGameService->destroyActor(pPlayer);

	PrintInfo("CPlayerMgr::destroyPlayer player_id: {} msg: {}", nPlayerID, szMsg);
}

uint32_t CPlayerMgr::getPlayerCount() const
{
	return (uint32_t)this->m_mapPlayer.size();
}

void CPlayerMgr::onGateDisconnect(uint32_t nGateServiceID)
{
	std::vector<uint64_t> vecPlayerID;
	for (auto iter = this->m_mapPlayer.begin(); iter != this->m_mapPlayer.end(); ++iter)
	{
		CPlayer* pPlayer = iter->second;
		if (nullptr == pPlayer)
			continue;

		if (pPlayer->getGateServiceID() != nGateServiceID)
			continue;

		uint32_t nUCServiceID = pPlayer->getUCServiceID();

		pPlayer->onPlayerLogout();
		vecPlayerID.push_back(pPlayer->getActorID());
		
		s2u_player_leave_notify notify_msg;
		notify_msg.set_player_id(pPlayer->getActorID());
		this->m_pGameService->getServiceInvoker()->send(nUCServiceID, &notify_msg);
	}

	for (size_t i = 0; i < vecPlayerID.size(); ++i)
	{
		this->destroyPlayer(vecPlayerID[i], "gate disconnect");
	}
}