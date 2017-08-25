#include "player_module.h"
#include "player.h"

CPlayerModule::CPlayerModule(CPlayer *pPlayer)
	: m_pPlayer(pPlayer)
{

}

CPlayerModule::~CPlayerModule()
{

}

uint64_t CPlayerModule::getPlayerID() const
{
	return this->m_pPlayer->getID();
}

CPlayer* CPlayerModule::getPlayer() const
{
	return this->m_pPlayer;
}