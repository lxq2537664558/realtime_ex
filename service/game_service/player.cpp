#include "player.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/base_app.h"
#include "libBaseCommon/string_util.h"

#include "player_attribute_module.h"
#include "player_hero_module.h"
#include "player_item_module.h"
#include "player_battlearray_module.h"
#include "game_service.h"

#include "../common/common.h"

#include "client_proto_src/s2c_player_login_complete_notify.pb.h"
#include "server_proto_src/s2u_player_heartbeat_request.pb.h"
#include "server_proto_src/s2u_player_heartbeat_response.pb.h"

using namespace core;

#define _PLAYER_BACKUP_TIME 10*1000

CPlayer::CPlayer()
	: m_eStatus(ePST_None)
	, m_nGateServiceID(0)
	, m_pServiceInvokeHolder(nullptr)
	, m_pDbServiceInvokeHolder(nullptr)
{
	this->m_tickHeartbeat.setCallback(std::bind(&CPlayer::onHeartbeat, this, std::placeholders::_1));
	this->m_tickerBackup.setCallback(std::bind(&CPlayer::onBackup, this, std::placeholders::_1));

	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i] = nullptr;
	}
}

CPlayer::~CPlayer()
{

}

void CPlayer::onInit(const void* pContext)
{
	PrintInfo("CPlayer::onInit");
	DebugAst(pContext != nullptr);

	const SCreatePlayerContext* pCreatePlayerContext = reinterpret_cast<const SCreatePlayerContext*>(pContext);

	this->m_eStatus = ePST_Init;
	
	this->m_pServiceInvokeHolder = new CServiceInvokeHolder(this->getServiceBase());
	this->m_pDbServiceInvokeHolder = new CDbServiceInvokeHolder(this->getServiceBase(), this->getDbServiceID());

	this->m_nGateServiceID = pCreatePlayerContext->nGateServiceID;

	this->onHeartbeat(0);
	this->getServiceBase()->registerTicker(&this->m_tickHeartbeat, 5000, 5000, 0);

	this->m_zPlayerModule[ePMT_Attribute] = new CPlayerAttributeModule(this);
	this->m_zPlayerModule[ePMT_Item] = new CPlayerItemModule(this);
	this->m_zPlayerModule[ePMT_Hero] = new CPlayerHeroModule(this);
	this->m_zPlayerModule[ePMT_BattleArray] = new CPlayerBattleArrayModule(this);
}

void CPlayer::onDestroy()
{
	this->m_eStatus = ePST_Destroy;
	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onDestroy();
	}

	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		SAFE_DELETE(this->m_zPlayerModule[i]);
	}

	SAFE_DELETE(this->m_pServiceInvokeHolder);
	SAFE_DELETE(this->m_pDbServiceInvokeHolder);

	PrintInfo("CPlayer::onDestroy");
}

void CPlayer::release()
{
	delete this;
}

void CPlayer::onHeartbeat(uint64_t nContext)
{
	s2u_player_heartbeat_request request_msg;
	request_msg.set_player_id(this->getActorID());

	this->getServiceInvokeHolder()->async_invoke<s2u_player_heartbeat_response>(this->getUCServiceID(), &request_msg, [this](const s2u_player_heartbeat_response* pMessage, uint32_t nErrorType)
	{
		if (nErrorType == eRRT_OK)
			return;

		PrintInfo("CPlayer::onHeartbeat timeout player_id: {}", this->getDbID());
		CGameService* pGameService = dynamic_cast<CGameService*>(this->getServiceBase());
		DebugAst(pGameService != nullptr);

		pGameService->getPlayerMgr()->destroyPlayer(this->getActorID(), "heartbeat timeout");
	});
}

void CPlayer::onLoadData()
{
	DebugAst(this->m_eStatus == ePST_Init);

	this->m_eStatus = ePST_LoadData;

	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onLoadData();
	}
}

void CPlayer::onBackup(uint64_t nContext)
{
	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onBackup();
	}

	//core::CBaseApp::Inst()->doQuit();
}

void CPlayer::onPlayerLogin()
{
	this->m_eStatus = ePST_Login;

	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onPlayerLogin();
	}

	s2c_player_login_complete_notify notify_msg;
	notify_msg.set_context(0);
	this->sendClientMessage(&notify_msg);
	
	this->m_eStatus = ePST_Normal;

	if (this->m_tickerBackup.isRegister())
		this->getServiceBase()->unregisterTicker(&this->m_tickerBackup);

	this->registerTicker(&this->m_tickerBackup, _PLAYER_BACKUP_TIME, _PLAYER_BACKUP_TIME, 0);
}

void CPlayer::onPlayerLogout()
{
	this->m_eStatus = ePST_Logout;
	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onPlayerLogout();
	}

	if (this->m_tickerBackup.isRegister())
		this->getServiceBase()->unregisterTicker(&this->m_tickerBackup);

	this->onBackup(0);
}

EPlayerStatusType CPlayer::getStatus() const
{
	return this->m_eStatus;
}

void CPlayer::setGateServiceID(uint32_t nGateServiceID)
{
	this->m_nGateServiceID = nGateServiceID;
}

uint32_t CPlayer::getGateServiceID() const
{
	return this->m_nGateServiceID;
}

void CPlayer::sendClientMessage(const google::protobuf::Message* pMessage)
{
	if (this->m_eStatus < ePST_Login)
	{
		PrintWarning("player not load data can't send msg to client player_id: {}", this->getActorID());
		return;
	}

	SClientSessionInfo sClientSessionInfo;
	sClientSessionInfo.nGateServiceID = this->m_nGateServiceID;
	sClientSessionInfo.nSessionID = this->getActorID();
	this->getServiceBase()->getServiceInvoker()->send(sClientSessionInfo, pMessage);
}

IPlayerAttributeModule* CPlayer::getAttributeModule() const
{
	return dynamic_cast<IPlayerAttributeModule*>(this->m_zPlayerModule[ePMT_Attribute]);
}

IPlayerHeroModule* CPlayer::getHeroModule() const
{
	return dynamic_cast<IPlayerHeroModule*>(this->m_zPlayerModule[ePMT_Hero]);
}

IPlayerBattleArrayModule* CPlayer::getBattleArrayModule() const
{
	return dynamic_cast<IPlayerBattleArrayModule*>(this->m_zPlayerModule[ePMT_BattleArray]);
}

IPlayerItemModule* CPlayer::getItemModule() const
{
	return dynamic_cast<IPlayerItemModule*>(this->m_zPlayerModule[ePMT_Item]);
}

const std::string& CPlayer::getName() const
{
	return this->getAttributeModule()->getName();
}

const std::string& CPlayer::getAccountName() const
{
	return this->getAttributeModule()->getAccountName();
}

uint32_t CPlayer::getServerID() const
{
	return this->getAttributeModule()->getServerID();
}

int64_t CPlayer::getLastLoginTime() const
{
	return this->getAttributeModule()->getLastLoginTime();
}

int64_t CPlayer::getLastLogoutTime() const
{
	return this->getAttributeModule()->getLastLogoutTime();
}

uint32_t CPlayer::getDbID() const
{
	return _GET_PLAYER_DB_ID(this->getActorID());
}

uint32_t CPlayer::getUCServiceID() const
{
	return this->getDbID() + _UC_SERVICE_DELTA;
}

uint32_t CPlayer::getDbServiceID() const
{
	return this->getDbID() + _GAME_DB_SERVICE_DELTA;
}

CServiceInvokeHolder* CPlayer::getServiceInvokeHolder() const
{
	return this->m_pServiceInvokeHolder;
}

CDbServiceInvokeHolder* CPlayer::getDbServiceInvokeHolder() const
{
	return this->m_pDbServiceInvokeHolder;
}

void CPlayer::setModuleLoadDataError(uint32_t nModuleType)
{
	this->m_setModuleLoadDataError.insert(nModuleType);
}

bool CPlayer::hasModuleLoadDataError() const
{
	return !this->m_setModuleLoadDataError.empty();
}
