#include "player_mgr.h"
#include "gas_service.h"
#include "../common/common.h"

CPlayerMgr::CPlayerMgr(CGasService* pGasService)
	: CDbServiceInvokeHolder(pGasService)
	, m_pGasService(pGasService)
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

CPlayer* CPlayerMgr::createPlayer(uint64_t nPlayerID, const std::string& szToken)
{
	if (nullptr != this->getPlayer(nPlayerID))
		return nullptr;

	CActorBase* pActorBase = this->m_pGasService->createActor("CPlayer", nPlayerID, szToken);
	if (nullptr == pActorBase)
		return nullptr;

	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pActorBase);
	if (nullptr == pPlayer)
	{
		this->m_pGasService->destroyActor(pActorBase);
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

	return pPlayer;
}

void CPlayerMgr::destroyPlayer(uint64_t nPlayerID, const std::string& szMsg)
{
	auto iter = this->m_mapPlayer.find(nPlayerID);
	if (iter == this->m_mapPlayer.end())
		return;

	CPlayer* pPlayer = iter->second;
	this->m_mapPlayer.erase(iter);

	this->m_pGasService->destroyActor(pPlayer);

	PrintInfo("CPlayerMgr::destroyPlayer player_id: {} msg: {}", nPlayerID, szMsg);
}

uint32_t CPlayerMgr::getPlayerCount() const
{
	return (uint32_t)this->m_mapPlayer.size();
}
