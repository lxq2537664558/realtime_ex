#include "game_service.h"
#include "game_service_message_handler.h"
#include "player_message_handler.h"
#include "player.h"

#include "libCoreCommon/service_invoker.h"

#include "server_proto_src/s2d_online_count_notify.pb.h"

#define _WEB_SOCKET_

CGameService::CGameService(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
{

}

CGameService::~CGameService()
{
}

CActorFactory* CGameService::getActorFactory(const std::string& szType) const
{
	return this->m_pPlayerFactory.get();
}

void CGameService::release()
{
	delete this;

	google::protobuf::ShutdownProtobufLibrary();
}

CPlayerMgr* CGameService::getPlayerMgr() const
{
	return this->m_pPlayerMgr.get();
}

CGMCommandMgr* CGameService::getGMCommandMgr() const
{
	return this->m_pGMCommandMgr.get();
}

bool CGameService::onInit()
{
	this->m_pNormalProtobufSerializer = std::make_unique<CNormalProtobufSerializer>();
	this->m_pJsonProtobufSerializer = std::make_unique<CJsonProtobufSerializer>();

	this->setForwardMessageSerializer(this->m_pJsonProtobufSerializer.get());
	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer.get());

	this->setServiceMessageSerializer(0, eMST_Protobuf);

	this->m_pPlayerFactory = std::make_unique<CPlayerFactory>();
	this->m_pPlayerMgr = std::make_unique<CPlayerMgr>(this);
	this->m_pGMCommandMgr = std::make_unique<CGMCommandMgr>(this);
	this->m_pGameServiceMessageHandler = std::make_unique<CGameServiceMessageHandler>(this);
	this->m_pPlayerMessageHandler = std::make_unique<CPlayerMessageHandler>(this);
	this->m_pPlayerHeroMessageHandler = std::make_unique<CPlayerHeroMessageHandler>(this);
	this->m_pPlayerItemMessageHandler = std::make_unique<CPlayerItemMessageHandler>(this);
	this->m_pBaseStaticConfigMgr = std::make_unique<CBaseStaticConfigMgr>();
	if (!this->m_pBaseStaticConfigMgr->init())
	{
		PrintInfo("load static config error");
		return false;
	}

	this->setServiceConnectCallback(std::bind(&CGameService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));
	this->setServiceDisconnectCallback(std::bind(&CGameService::onServiceDisconnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyOnlineCount.setCallback(std::bind(&CGameService::onNotifyOnlineCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyOnlineCount, 5000, 5000, 0);

	PrintInfo("CGameService::onInit");

	return true;
}

void CGameService::onFrame()
{
	//PrintInfo("CGameService::onFrame");
}

void CGameService::onQuit()
{
	PrintInfo("CGameService::onQuit");
	this->doQuit();
}

void CGameService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		s2d_online_count_notify msg;
		msg.set_count(this->m_pPlayerMgr->getPlayerCount());
		this->getServiceInvoker()->send(nServiceID, &msg);
	}
}

void CGameService::onServiceDisconnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "gate")
	{
		this->m_pPlayerMgr->onGateDisconnect(nServiceID);
	}
}

void CGameService::onNotifyOnlineCount(uint64_t nContext)
{
	s2d_online_count_notify msg;
	msg.set_count(this->m_pPlayerMgr->getPlayerCount());
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CGameService(sServiceBaseInfo, szConfigFileName);
}