#include "libBaseCommon/string_util.h"
#include "libBaseCommon/time_util.h"

#include "player_attribute_module.h"
#include "player.h"

#include "../common/common.h"

#include "db_proto_src/player_base.pb.h"
#include "client_proto_src/s2c_player_attribute_notify.pb.h"
#include "client_proto_src/s2c_update_player_attribute_notify.pb.h"

CPlayerAttributeModule::CPlayerAttributeModule(CPlayer* pPlayer)
	: CPlayerModule(pPlayer)
{
	this->m_nLastLoginTime = 0;
	this->m_nLastLogoutTime = 0;
	this->m_nServerID = 0;
	this->m_nFace = 0;
	this->m_nLv = 0;
	this->m_nExp = 0;
	this->m_nGold = 0;
	this->m_nMoney = 0;
	this->m_nVitality = 0;

	this->m_zAddFunc[eAT_Lv]		= std::bind(&CPlayerAttributeModule::addLv, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	this->m_zAddFunc[eAT_Exp]		= std::bind(&CPlayerAttributeModule::addExp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	this->m_zAddFunc[eAT_Gold]		= std::bind(&CPlayerAttributeModule::addGold, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	this->m_zAddFunc[eAT_Money]		= std::bind(&CPlayerAttributeModule::addMoney, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	this->m_zAddFunc[eAT_Vitality]	= std::bind(&CPlayerAttributeModule::addVitality, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	this->m_zGetFunc[eAT_Face]		= std::bind(&CPlayerAttributeModule::getFace, this);
	this->m_zGetFunc[eAT_Lv]		= std::bind(&CPlayerAttributeModule::getLv, this);
	this->m_zGetFunc[eAT_Exp]		= std::bind(&CPlayerAttributeModule::getExp, this);
	this->m_zGetFunc[eAT_Gold]		= std::bind(&CPlayerAttributeModule::getGold, this);
	this->m_zGetFunc[eAT_Money]		= std::bind(&CPlayerAttributeModule::getMoney, this);
	this->m_zGetFunc[eAT_Vitality]	= std::bind(&CPlayerAttributeModule::getVitality, this);
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
	this->m_pPlayer->getDbServiceInvokeHolder()->async_select(this->getPlayerID(), "player_base", [this](const google::protobuf::Message* pMessage, uint32_t nErrorCode)
	{
		if (nErrorCode == base::db::eDBRC_EmptyRecordset)
			return;

		const proto::db::player_base* pPlayerBase = dynamic_cast<const proto::db::player_base*>(pMessage);
		if (nullptr == pPlayerBase)
		{
			this->m_pPlayer->setModuleLoadDataError(this->getModuleType());

			PrintInfo("CPlayerAttributeModule::onLoadData error player_id: {}", this->getPlayerID());
			return;
		}

		this->m_nLastLoginTime = pPlayerBase->last_login_time();
		this->m_nLastLogoutTime = pPlayerBase->last_logout_time();
		this->m_szAccountName = pPlayerBase->account_name();
		this->m_nServerID = pPlayerBase->server_id();
		this->m_szName = pPlayerBase->name();
		this->m_nFace = pPlayerBase->face();
		this->m_nLv = pPlayerBase->lv();
		this->m_nExp = pPlayerBase->exp();
		this->m_nGold = pPlayerBase->gold();
		this->m_nMoney = pPlayerBase->money();
		this->m_nVitality = pPlayerBase->vitality();

		if (this->m_szName.empty())
			this->m_szName = base::string_util::convert_to_str(this->getPlayerID());
	});
}

void CPlayerAttributeModule::onBackup()
{
	proto::db::player_base db_msg;

	db_msg.set_player_id(this->getPlayerID());
	db_msg.set_last_login_time(this->m_nLastLoginTime);
	db_msg.set_last_logout_time(this->m_nLastLogoutTime);
	db_msg.set_name(this->m_szName);
	db_msg.set_face(this->m_nFace);
	db_msg.set_lv(this->m_nLv);
	db_msg.set_exp(this->m_nExp);
	db_msg.set_gold(this->m_nGold);
	db_msg.set_money(this->m_nMoney);
	db_msg.set_vitality(this->m_nVitality);

	this->m_pPlayer->getDbServiceInvokeHolder()->update(&db_msg);
}

void CPlayerAttributeModule::onPlayerLogin()
{
	this->m_nLastLoginTime = base::time_util::getGmtTime();

	s2c_player_attribute_notify notify_msg;
	notify_msg.set_name(this->m_szName);
	notify_msg.set_face(this->m_nFace);
	notify_msg.set_lv(this->m_nLv);
	notify_msg.set_exp(this->m_nExp);
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

uint32_t CPlayerAttributeModule::getFace() const
{
	return this->m_nFace;
}

const std::string& CPlayerAttributeModule::getName() const
{
	return this->m_szName;
}

const std::string& CPlayerAttributeModule::getAccountName() const
{
	return this->m_szAccountName;
}

uint32_t CPlayerAttributeModule::getServerID() const
{
	return this->m_nServerID;
}

int64_t CPlayerAttributeModule::getLastLoginTime() const
{
	return this->m_nLastLoginTime;
}

uint32_t CPlayerAttributeModule::getLv() const
{
	return this->m_nLv;
}

uint64_t CPlayerAttributeModule::getExp() const
{
	return this->m_nExp;
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

int64_t CPlayerAttributeModule::getLastLogoutTime() const
{
	return this->m_nLastLogoutTime;
}

int64_t CPlayerAttributeModule::getAttribute(uint32_t nType) const
{
	DebugAstEx(nType < eAT_Count, 0);

	auto& func = this->m_zGetFunc[nType];
	if (nullptr == func)
		return 0;

	return func();
}

void CPlayerAttributeModule::setAttribute(uint32_t nType, int64_t nData, uint32_t nActionType, const void* pContext)
{
	DebugAst(nType < eAT_Count);

	int64_t nOldData = this->getAttribute(nType);
	int64_t nModifyData = nOldData - nData;
	this->addAttribute(nType, nModifyData, nActionType, pContext);
}

void CPlayerAttributeModule::addAttribute(uint32_t nType, int64_t nData, uint32_t nActionType, const void* pContext)
{
	DebugAst(nType < eAT_Count);

	auto& func = this->m_zAddFunc[nType];
	if (nullptr == func)
		return;

	func(nData, nActionType, pContext);
}

void CPlayerAttributeModule::addLv(int64_t nData, uint32_t nActionType /* = 0 */, const void* pContext /* = nullptr */)
{
	int64_t nLv = this->m_nLv;
	nLv += nData;
	DebugAst(nLv > 0);

	this->m_nLv = (uint32_t)nLv;

	this->syncAttribute(eAT_Lv, this->m_nLv);

	CPlayerLvupEvent event;
	this->m_pPlayer->raiseEvent(&event);
}

void CPlayerAttributeModule::addExp(int64_t nData, uint32_t nActionType /*= 0*/, const void* pContext /*= nullptr*/)
{
	int64_t nExp = this->m_nExp;
	nExp += nData;
	DebugAst(nExp > 0);

	this->m_nExp = (uint64_t)nExp;

	this->syncAttribute(eAT_Exp, this->m_nExp);
}

void CPlayerAttributeModule::addGold(int64_t nData, uint32_t nActionType /* = 0 */, const void* pContext /* = nullptr */)
{
	int64_t nGold = this->m_nGold;
	nGold += nData;
	DebugAst(nGold > 0);

	this->m_nGold = (uint64_t)nGold;

	this->syncAttribute(eAT_Gold, this->m_nGold);
}

void CPlayerAttributeModule::addMoney(int64_t nData, uint32_t nActionType /*= 0*/, const void* pContext /*= nullptr*/)
{
	int64_t nMoney = this->m_nMoney;
	nMoney += nData;
	DebugAst(nMoney > 0);

	this->m_nMoney = (uint64_t)nMoney;

	this->syncAttribute(eAT_Money, this->m_nMoney);
}

void CPlayerAttributeModule::addVitality(int64_t nData, uint32_t nActionType /*= 0*/, const void* pContext /*= nullptr*/)
{
	int64_t nVitality = this->m_nVitality;
	nVitality += nData;
	DebugAst(nVitality > 0);

	this->m_nVitality = (uint64_t)nVitality;

	this->syncAttribute(eAT_Vitality, this->m_nVitality);
}

void CPlayerAttributeModule::syncAttribute(uint32_t nType, uint64_t nData)
{
	s2c_update_player_attribute_notify notify_msg;
	notify_msg.set_type(nType);
	notify_msg.set_value(nData);

	this->getPlayer()->sendClientMessage(&notify_msg);
}
