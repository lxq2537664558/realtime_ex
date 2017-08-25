#include "player_attribute_module.h"
#include "player.h"

#include "../common/common.h"

#include "libBaseCommon/string_util.h"

#include "db_proto_src/player_base.pb.h"
#include "msg_proto_src/s2c_player_attribute_notify.pb.h"
#include "libBaseCommon/time_util.h"

CPlayerAttributeModule::CPlayerAttributeModule(CPlayer* pPlayer)
	: CPlayerModule(pPlayer)
	, CDbServiceInvokeHolder(pPlayer->getServiceBase(), _GET_PLAYER_DB_ID(pPlayer->getID()) + _GAME_DB_SERVICE_DELTA)
{

}

CPlayerAttributeModule::~CPlayerAttributeModule()
{

}

void CPlayerAttributeModule::onInit()
{

}

void CPlayerAttributeModule::onDestroy()
{

}

void CPlayerAttributeModule::onLoadData()
{
	this->async_select(this->getPlayerID(), "player_base", [this](const google::protobuf::Message* pMessage, uint32_t nErrorCode)
	{
		const proto::db::player_base* pPlayerBase = dynamic_cast<const proto::db::player_base*>(pMessage);
		if (nullptr == pPlayerBase)
		{
			PrintInfo("CPlayerAttributeModule::onLoadData error player_id: {}", this->getPlayerID());
			return;
		}

		this->m_nLastLoginTime = pPlayerBase->last_login_time();
		this->m_nLastLogoutTime = pPlayerBase->last_logout_time();
		this->m_szAccountName = pPlayerBase->account_name();
		this->m_nServerID = pPlayerBase->server_id();
		this->m_szName = pPlayerBase->name();
		this->m_nLv = pPlayerBase->lv();
		this->m_nGold = pPlayerBase->gold();
		this->m_nMoney = pPlayerBase->money();
		this->m_nVitality = pPlayerBase->vitality();

		if (this->m_szName.empty())
			this->m_szName = base::string_util::convert_to_str(this->getPlayerID());
	});
}

void CPlayerAttributeModule::onBackup()
{
	proto::db::player_base player_base;

	player_base.set_player_id(this->getPlayerID());
	player_base.set_last_login_time(this->m_nLastLoginTime);
	player_base.set_last_logout_time(this->m_nLastLogoutTime);
	player_base.set_name(this->m_szName);
	player_base.set_lv(this->m_nLv);
	player_base.set_gold(this->m_nGold);
	player_base.set_money(this->m_nMoney);
	player_base.set_vitality(this->m_nVitality);

	this->update(&player_base);
}

void CPlayerAttributeModule::onPlayerLogin()
{
	this->m_nLastLoginTime = base::time_util::getGmtTime();

	s2c_player_attribute_notify notify_msg;
	notify_msg.set_name(this->m_szName);
	notify_msg.set_lv(this->m_nLv);
	notify_msg.set_gold(this->m_nGold);
	notify_msg.set_money(this->m_nMoney);
	notify_msg.set_vitality(this->m_nVitality);

	this->m_pPlayer->sendClientMessage(&notify_msg);
}

void CPlayerAttributeModule::onPlayerLogout()
{
	this->m_nLastLogoutTime = base::time_util::getGmtTime();
}

void CPlayerAttributeModule::onModuleEvent(const IPlayerModuleEvent* pEvent)
{

}

const std::string& CPlayerAttributeModule::getName() const
{
	return this->m_szName;
}

uint32_t CPlayerAttributeModule::getLv() const
{
	return this->m_nLv;
}

uint64_t CPlayerAttributeModule::getGold() const
{
	return this->m_nGold;
}

uint64_t CPlayerAttributeModule::getMoney() const
{
	return this->m_nMoney;
}

uint64_t CPlayerAttributeModule::getVitality() const
{
	return this->m_nVitality;
}
