#pragma once
#include "libBaseCommon/ticker.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/normal_protobuf_serializer.h"
#include "libCoreCommon/json_protobuf_serializer.h"

#include "player_mgr.h"
#include "gm_command_mgr.h"
#include "base_static_config_mgr.h"
#include "player_hero_message_handler.h"
#include "player_message_handler.h"
#include "game_service_message_handler.h"
#include "player_item_message_handler.h"

using namespace std;
using namespace core;
using namespace base;

class CGameService :
	public CServiceBase
{
public:
	CGameService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
	virtual ~CGameService();

	virtual void	release();

	CPlayerMgr*		getPlayerMgr() const;
	CGMCommandMgr*	getGMCommandMgr() const;

private:
	virtual bool	onInit();
	virtual void	onFrame();
	virtual void	onQuit();

	void			onServiceConnect(const std::string& szType, uint32_t nServiceID);
	void			onServiceDisconnect(const std::string& szType, uint32_t nServiceID);
	void			onNotifyOnlineCount(uint64_t nContext);

private:
	std::unique_ptr<CNormalProtobufSerializer>	m_pNormalProtobufSerializer;
	std::unique_ptr<CJsonProtobufSerializer>	m_pJsonProtobufSerializer;
	std::unique_ptr<CPlayerMgr>					m_pPlayerMgr;
	std::unique_ptr<CBaseStaticConfigMgr>		m_pBaseStaticConfigMgr;
	std::unique_ptr<CGMCommandMgr>				m_pGMCommandMgr;

	std::unique_ptr<CGameServiceMessageHandler>	m_pGameServiceMessageHandler;
	std::unique_ptr<CPlayerHeroMessageHandler>	m_pPlayerHeroMessageHandler;
	std::unique_ptr<CPlayerItemMessageHandler>	m_pPlayerItemMessageHandler;
	std::unique_ptr<CPlayerMessageHandler>		m_pPlayerMessageHandler;
	
	base::CTicker								m_tickerNotifyOnlineCount;
};