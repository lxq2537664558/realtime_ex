#pragma once

#include "player_battlearray_module_interface.h"
#include "player_module.h"
#include "player.h"

#include <map>

class CPlayerBattleArrayModule
	: public IPlayerBattleArrayModule
	, public CPlayerModule
{
public:
	CPlayerBattleArrayModule(CPlayer* pPlayer);
	virtual ~CPlayerBattleArrayModule();

	virtual void					onInit();
	virtual void					onDestroy();
	virtual void					onLoadData();
	virtual void					onBackup();
	virtual void					onPlayerLogin();
	virtual void					onPlayerLogout();
	virtual void					onModuleEvent(const IPlayerModuleEvent* pEvent);
	virtual uint32_t				getModuleType() const { return ePMT_BattleArray; }

	virtual const SBattleArrayInfo*	getBattleArrayInfo(EBattleArrayType eType);
	virtual void					updateBattleArrayInfo(EBattleArrayType eType, const SBattleArrayInfo& sBattleArrayInfo);

private:
	std::map<uint32_t, SBattleArrayInfo>	m_mapBattleArrayInfo;
};