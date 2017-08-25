#include "player.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"

#include "msg_proto_src/s2u_player_heartbeat_notify.pb.h"
#include "../common/common.h"

#include "player_attribute_module.h"
#include "libBaseCommon/string_util.h"
#include "msg_proto_src/s2c_player_login_complete_notify.pb.h"

using namespace core;

CPlayer::CPlayer()
	: m_eStatus(ePST_None)
	, m_nGateServiceID(0)
{
	this->m_tickHeartbeat.setCallback(std::bind(&CPlayer::onHeartbeat, this, std::placeholders::_1));

	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i] = nullptr;
	}
}

CPlayer::~CPlayer()
{

}

void CPlayer::onInit(const std::string& szContext)
{
	PrintInfo("CPlayer::onInit");

	this->m_eStatus = ePST_Init;
	
	base::string_util::convert_to_value(szContext, this->m_nGateServiceID);
	
	this->onHeartbeat(0);
	this->getServiceBase()->registerTicker(&this->m_tickHeartbeat, 5000, 5000, 0);

	this->m_zPlayerModule[ePMT_Attribute] = new CPlayerAttributeModule(this);
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

	PrintInfo("CPlayer::onDestroy");
}

void CPlayer::release()
{
	delete this;
}

void CPlayer::onHeartbeat(uint64_t nContext)
{
	s2u_player_heartbeat_notify notify_msg;
	notify_msg.set_player_id(this->getID());

	uint32_t nDBID = _GET_PLAYER_DB_ID(this->getID());
	uint32_t nUCServiceID = nDBID + _UC_SERVICE_DELTA;
	this->getServiceBase()->getServiceInvoker()->send(eMTT_Service, nUCServiceID, &notify_msg);
}

void CPlayer::onLoadData()
{
	this->m_eStatus = ePST_LoadData;

	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onLoadData();
	}
}

void CPlayer::onBackup()
{
	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onBackup();
	}
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
}

void CPlayer::onPlayerLogout()
{
	this->m_eStatus = ePST_Logout;
	for (size_t i = 0; i < _countof(this->m_zPlayerModule); ++i)
	{
		this->m_zPlayerModule[i]->onPlayerLogout();
	}

	this->onBackup();
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
	SClientSessionInfo sClientSessionInfo;
	sClientSessionInfo.nGateServiceID = this->m_nGateServiceID;
	sClientSessionInfo.nSessionID = this->getID();
	this->getServiceBase()->getServiceInvoker()->send(sClientSessionInfo, pMessage);
}