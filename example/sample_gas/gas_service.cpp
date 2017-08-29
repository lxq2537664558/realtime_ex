#include "gas_service.h"
#include "gas_service_message_handler.h"
#include "player_message_handler.h"
#include "player.h"

#include "libCoreCommon/service_invoker.h"

#include "msg_proto_src/s2d_online_count_notify.pb.h"

//#define _WEB_SOCKET_

CGasService::CGasService()
	: m_pGasServiceMessageHandler(nullptr)
	, m_pPlayerMessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
	, m_pPlayerFactory(nullptr)
	, m_pJsonProtobufFactory(nullptr)
	, m_pPlayerMgr(nullptr)
{

}

CGasService::~CGasService()
{
	SAFE_DELETE(this->m_pNormalProtobufFactory);
	SAFE_DELETE(this->m_pJsonProtobufFactory);
}

core::CProtobufFactory* CGasService::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

core::CProtobufFactory* CGasService::getForwardProtobufFactory() const
{
#ifdef _WEB_SOCKET_
	return this->m_pJsonProtobufFactory;
#else
	return this->m_pNormalProtobufFactory;
#endif
}

CActorFactory* CGasService::getActorFactory(const std::string& szType) const
{
	return this->m_pPlayerFactory;
}

void CGasService::release()
{
	delete this;
}

CPlayerMgr* CGasService::getPlayerMgr() const
{
	return this->m_pPlayerMgr;
}

bool CGasService::onInit()
{
	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	this->m_pJsonProtobufFactory = new CJsonProtobufFactory();

	this->m_pPlayerFactory = new CPlayerFactory();
	this->m_pPlayerMgr = new CPlayerMgr(this);
	this->m_pGasServiceMessageHandler = new CGasServiceMessageHandler(this);
	this->m_pPlayerMessageHandler = new CPlayerMessageHandler(this);

	this->setServiceConnectCallback(std::bind(&CGasService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyOnlineCount.setCallback(std::bind(&CGasService::onNotifyOnlineCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyOnlineCount, 5000, 5000, 0);

	PrintInfo("CWatchdogService::onInit");

	return true;
}

void CGasService::onFrame()
{
	//PrintInfo("CWatchdogService::onFrame");
}

void CGasService::onQuit()
{
	PrintInfo("CWatchdogService::onQuit");
}

void CGasService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		s2d_online_count_notify msg;
		msg.set_count(this->m_pPlayerMgr->getPlayerCount());
		this->getServiceInvoker()->send(nServiceID, &msg);
	}
}

void CGasService::onNotifyOnlineCount(uint64_t nContext)
{
	s2d_online_count_notify msg;
	msg.set_count(this->m_pPlayerMgr->getPlayerCount());
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CGasService();
}