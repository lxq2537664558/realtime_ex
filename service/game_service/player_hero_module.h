#pragma once

#include "player_hero_module_interface.h"
#include "player_module.h"
#include "player.h"

#include <set>

class CPlayerHeroModule
	: public IPlayerHeroModule
	, public CPlayerModule
{
public:
	CPlayerHeroModule(CPlayer* pPlayer);
	virtual ~CPlayerHeroModule();

	virtual void		onInit();
	virtual void		onDestroy();
	virtual void		onLoadData();
	virtual void		onBackup();
	virtual void		onPlayerLogin();
	virtual void		onPlayerLogout();
	virtual void		onModuleEvent(const IPlayerModuleEvent* pEvent);
	virtual uint32_t	getModuleType() const { return ePMT_Hero; }

	virtual bool		isPatchActive(uint32_t nID) const;
	virtual bool		isHeroActive(uint32_t nID) const;

	virtual void		activePatch(uint32_t nID);
	virtual void		activeHero(uint32_t nID);

private:
	std::set<uint32_t>	m_setActivePatchID;
	std::set<uint32_t>	m_setActiveHeroID;
};