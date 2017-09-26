#pragma once
#include "player_battlearray_module_interface.h"
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/actor_base.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/service_invoke_holder.h"
#include "libCoreCommon/db_service_invoke_holder.h"

#include "player_module.h"
#include "event_dispatcher.h"
#include "player_attribute_module_interface.h"
#include "player_hero_module_interface.h"
#include "player_item_module_interface.h"

enum EPlayerModuleType
{
	ePMT_Attribute		= 0,
	ePMT_Item			= 1,
	ePMT_Hero			= 2,
	ePMT_BattleArray	= 3,
	ePMT_Count,
};

enum EPlayerStatusType
{
	ePST_None,
	ePST_Init,
	ePST_LoadData,
	ePST_Login,
	ePST_Normal,
	ePST_Logout,
	ePST_Destroy,
};

struct SCreatePlayerContext
{
	uint32_t	nGateServiceID;
};

class CGameServiceMessageHandler;
class CPlayerMgr;
class CPlayer :
	public core::CActorBase,
	public CEventDispatcher
{
	friend class CGameServiceMessageHandler;
	friend class CPlayerMgr;

public:
	CPlayer();
	virtual ~CPlayer();

	EPlayerStatusType	getStatus() const;
	void				setGateServiceID(uint32_t nGateServiceID);
	uint32_t			getGateServiceID() const;

	void				sendClientMessage(const google::protobuf::Message* pMessage);

	const std::string&	getName() const;
	const std::string&	getAccountName() const;
	uint32_t			getServerID() const;
	int64_t				getLastLoginTime() const;
	int64_t				getLastLogoutTime() const;

	core::CServiceInvokeHolder*		getServiceInvokeHolder() const;
	core::CDbServiceInvokeHolder*	getDbServiceInvokeHolder() const;

	uint32_t	getDbID() const;
	uint32_t	getUCServiceID() const;
	uint32_t	getDbServiceID() const;

	IPlayerAttributeModule*		getAttributeModule() const;
	IPlayerItemModule*			getItemModule() const;
	IPlayerHeroModule*			getHeroModule() const;
	IPlayerBattleArrayModule*	getBattleArrayModule() const;

	void					setModuleLoadDataError(uint32_t nModuleType);
	bool					hasModuleLoadDataError() const;

private:
	virtual void	onInit(const void* pContext);
	virtual void	onDestroy();

	virtual void	release();

	void			onLoadData();
	void			onBackup(uint64_t nContext);
	void			onPlayerLogin();
	void			onPlayerLogout();

	void			onHeartbeat(uint64_t nContext);

private:
	core::CTicker					m_tickHeartbeat;
	EPlayerStatusType				m_eStatus;
	uint32_t						m_nGateServiceID;
	CPlayerModule*					m_zPlayerModule[ePMT_Count];
	core::CServiceInvokeHolder*		m_pServiceInvokeHolder;
	core::CDbServiceInvokeHolder*	m_pDbServiceInvokeHolder;

	std::set<uint32_t>				m_setModuleLoadDataError;

	core::CTicker					m_tickerBackup;
};