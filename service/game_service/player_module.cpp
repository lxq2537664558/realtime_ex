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
	return this->m_pPlayer->getPlayerID();
}

CPlayer* CPlayerModule::getPlayer() const
{
	return this->m_pPlayer;
}

void CPlayerModule::onEvent(const IEvent* pEvent)
{
	DebugAst(pEvent != nullptr);

	const IPlayerModuleEvent* pPlayerModuleEvent = dynamic_cast<const IPlayerModuleEvent*>(pEvent);
	DebugAst(pPlayerModuleEvent != nullptr);

	this->onModuleEvent(pPlayerModuleEvent);
}