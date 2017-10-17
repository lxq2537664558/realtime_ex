#pragma once
#include "player_module_event.h"
#include "event_handler.h"

class CPlayer;
class CPlayerModule 
	: public IEventHandler
{
public:
	CPlayerModule(CPlayer *pPlayer);
	virtual ~CPlayerModule();

	virtual void		onInit() { }	// 初始化数据(此时还不能跟客户端通讯)
	virtual void		onDestroy() { }	// 释放内存
	virtual void		onLoadData() { }	// 加载玩家数据(此时还不能跟客户端通讯)
	virtual void		onBackup() { }	// 数据备份
	virtual void		onPlayerLogin() { }	// 玩家上线(数据准备完毕 正式加入游戏，此时能跟客户端通讯)
	virtual void		onPlayerLogout() { }	// 玩家下线
	virtual void		onModuleEvent(const IPlayerModuleEvent* pEvent) { }	// 玩家模块事件

	virtual uint32_t	getModuleType() const = 0;

	uint64_t			getPlayerID() const;
	CPlayer*			getPlayer() const;

private:
	virtual void		onEvent(const IEvent* pEvent);

protected:
	CPlayer*	m_pPlayer;
};