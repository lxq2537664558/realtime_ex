#include "libBaseCommon/string_util.h"
#include "libBaseCommon/time_util.h"

#include "player_battlearray_module.h"
#include "player.h"

#include "../common/common.h"

#include "db_proto_src/player_item.pb.h"
#include "client_proto_src/s2c_player_item_notify.pb.h"
#include "client_proto_src/s2c_add_player_item_notify.pb.h"
#include "client_proto_src/s2c_del_player_item_notify.pb.h"

CPlayerBattleArrayModule::CPlayerBattleArrayModule(CPlayer* pPlayer)
	: CPlayerModule(pPlayer)
{

}

CPlayerBattleArrayModule::~CPlayerBattleArrayModule()
{

}

void CPlayerBattleArrayModule::onInit()
{

}

void CPlayerBattleArrayModule::onDestroy()
{

}

void CPlayerBattleArrayModule::onLoadData()
{
}

void CPlayerBattleArrayModule::onBackup()
{
	
}

void CPlayerBattleArrayModule::onPlayerLogin()
{
	
}

void CPlayerBattleArrayModule::onPlayerLogout()
{

}

void CPlayerBattleArrayModule::onModuleEvent(const IPlayerModuleEvent* pEvent)
{

}

const SBattleArrayInfo* CPlayerBattleArrayModule::getBattleArrayInfo(EBattleArrayType eType)
{
	auto iter = this->m_mapBattleArrayInfo.find(eType);
	if (iter == this->m_mapBattleArrayInfo.end())
		return nullptr;

	return &iter->second;
}

void CPlayerBattleArrayModule::updateBattleArrayInfo(EBattleArrayType eType, const SBattleArrayInfo& sBattleArrayInfo)
{

}
