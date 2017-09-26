#pragma once

#include "player_item_module_interface.h"
#include "player_module.h"
#include "player.h"

#include <map>

class CPlayerItemModule
	: public IPlayerItemModule
	, public CPlayerModule
{
public:
	CPlayerItemModule(CPlayer* pPlayer);
	virtual ~CPlayerItemModule();

	virtual void		onInit();
	virtual void		onDestroy();
	virtual void		onLoadData();
	virtual void		onBackup();
	virtual void		onPlayerLogin();
	virtual void		onPlayerLogout();
	virtual void		onModuleEvent(const IPlayerModuleEvent* pEvent);
	virtual uint32_t	getModuleType() const { return ePMT_Item; }

	virtual void		addItem(uint32_t nID, uint32_t nCount, uint32_t nActionType = 0, const void* pContext = nullptr);
	virtual void		delItem(uint32_t nID, uint32_t nCount, uint32_t nActionType = 0, const void* pContext = nullptr);
	virtual uint32_t	getItemCount(uint32_t nID) const;

private:
	std::map<uint32_t, uint32_t>	m_mapItem;
};