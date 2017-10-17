#include "libBaseCommon/string_util.h"
#include "libBaseCommon/time_util.h"

#include "player_hero_module.h"
#include "player.h"

#include "../common/common.h"

#include "db_proto_src/player_hero.pb.h"
#include "client_proto_src/s2c_player_hero_notify.pb.h"
#include "client_proto_src/s2c_active_player_hero_notify.pb.h"
#include "client_proto_src/s2c_active_player_hero_patch_notify.pb.h"

CPlayerHeroModule::CPlayerHeroModule(CPlayer* pPlayer)
	: CPlayerModule(pPlayer)
{

}

CPlayerHeroModule::~CPlayerHeroModule()
{

}

void CPlayerHeroModule::onInit()
{

}

void CPlayerHeroModule::onDestroy()
{

}

void CPlayerHeroModule::onLoadData()
{
	this->m_pPlayer->getDbServiceInvokeHolder()->async_select(this->getPlayerID(), "player_hero", [this](const google::protobuf::Message* pMessage, uint32_t nErrorCode)
	{
		if (nErrorCode == base::db::eDBRC_EmptyRecordset)
			return;
		
		const proto::db::player_hero* pPlayerHero = dynamic_cast<const proto::db::player_hero*>(pMessage);
		if (nullptr == pPlayerHero)
		{
			this->m_pPlayer->setModuleLoadDataError(this->getModuleType());

			PrintInfo("CPlayerHeroModule::onLoadData error player_id: {}", this->getPlayerID());
			return;
		}

		for (int32_t i = 0; i < pPlayerHero->hero_data().data_size(); ++i)
		{
			const proto::db::player_hero_data& sHero = pPlayerHero->hero_data().data(i);
			this->m_setActiveHeroID.insert(sHero.id());
		}

		for (int32_t i = 0; i < pPlayerHero->patch_data().data_size(); ++i)
		{
			const proto::db::player_hero_patch_data& sHeroPatch = pPlayerHero->patch_data().data(i);
			this->m_setActivePatchID.insert(sHeroPatch.id());
		}
	});
}

void CPlayerHeroModule::onBackup()
{
	proto::db::player_hero db_msg;
	db_msg.set_player_id(this->getPlayerID());
	for (auto iter = this->m_setActiveHeroID.begin(); iter != this->m_setActiveHeroID.end(); ++iter)
	{
		proto::db::player_hero_data* pHero = db_msg.mutable_hero_data()->add_data();
		DebugAst(pHero != nullptr);

		pHero->set_id(*iter);
	}

	for (auto iter = this->m_setActivePatchID.begin(); iter != this->m_setActivePatchID.end(); ++iter)
	{
		proto::db::player_hero_patch_data* pHeroPatch = db_msg.mutable_patch_data()->add_data();
		DebugAst(pHeroPatch != nullptr);

		pHeroPatch->set_id(*iter);
	}

	this->m_pPlayer->getDbServiceInvokeHolder()->update(&db_msg);
}

void CPlayerHeroModule::onPlayerLogin()
{
	s2c_player_hero_notify notify_msg;
	for (auto iter = this->m_setActiveHeroID.begin(); iter != this->m_setActiveHeroID.end(); ++iter)
	{
		auto* pHero = notify_msg.add_hero_data();
		DebugAst(pHero != nullptr);

		pHero->set_id(*iter);
	}

	for (auto iter = this->m_setActivePatchID.begin(); iter != this->m_setActivePatchID.end(); ++iter)
	{
		auto* pHeroPatch = notify_msg.add_patch_data();
		DebugAst(pHeroPatch != nullptr);

		pHeroPatch->set_id(*iter);
	}

	this->m_pPlayer->sendClientMessage(&notify_msg);
}

void CPlayerHeroModule::onPlayerLogout()
{
	
}

void CPlayerHeroModule::onModuleEvent(const IPlayerModuleEvent* pEvent)
{

}

bool CPlayerHeroModule::isPatchActive(uint32_t nID) const
{
	return this->m_setActivePatchID.find(nID) != this->m_setActivePatchID.end();
}

bool CPlayerHeroModule::isHeroActive(uint32_t nID) const
{
	return this->m_setActiveHeroID.find(nID) != this->m_setActiveHeroID.end();
}

void CPlayerHeroModule::activePatch(uint32_t nID)
{
	if (this->isPatchActive(nID))
		return;

	this->m_setActivePatchID.insert(nID);

	PrintInfo("CPlayerHeroModule::activePatch player_id: {} hero_id: {}", this->getPlayerID(), nID);

	s2c_active_player_hero_patch_notify notify_msg;
	notify_msg.set_id(nID);
	this->m_pPlayer->sendClientMessage(&notify_msg);

	CPlayerActiveHeroPatchEvent event;
	event.nHeroPatchID = nID;
	this->m_pPlayer->raiseEvent(&event);
}

void CPlayerHeroModule::activeHero(uint32_t nID)
{
	if (this->isHeroActive(nID))
		return;

	this->m_setActiveHeroID.insert(nID);

	PrintInfo("CPlayerHeroModule::activeHero player_id: {} hero_id: {}", this->getPlayerID(), nID);

	s2c_active_player_hero_notify notify_msg;
	notify_msg.set_id(nID);
	this->m_pPlayer->sendClientMessage(&notify_msg);

	CPlayerActiveHeroEvent event;
	event.nHeroID = nID;
	this->m_pPlayer->raiseEvent(&event);
}