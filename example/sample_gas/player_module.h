#pragma once
#include "player_module_event.h"

class CPlayer;
class CPlayerModule
{
public:
	CPlayerModule(CPlayer *pPlayer);
	virtual ~CPlayerModule();

	virtual void	onInit() { }										// 初始化数据
	virtual void	onDestroy() { }										// 释放内存
	virtual void    onLoadData() { }									// 加载玩家数据
	virtual void	onBackup() { }										// 数据备份
	virtual void	onPlayerLogin() { }									// 玩家上线[数据准备完毕 正式加入游戏]
	virtual void	onPlayerLogout() { }								// 玩家下线
	virtual void	onModuleEvent(const IPlayerModuleEvent* pEvent) { }	// 玩家模块时间

	uint64_t		getPlayerID() const;
	CPlayer*		getPlayer() const;

protected:
	CPlayer*	m_pPlayer;
};