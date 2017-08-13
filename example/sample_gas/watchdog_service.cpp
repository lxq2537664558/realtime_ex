#include "watchdog_service.h"
#include "watchdog_service_message_handler.h"
#include "player_message_handler.h"
#include "player.h"

#include "proto_src/gas_online_count_notify.pb.h"

CWatchdogService::CWatchdogService()
	: m_pWatchdogServiceMessageHandler(nullptr)
	, m_pPlayerMessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
	, m_pPlayerFactory(nullptr)
	, m_pJsonProtobufFactory(nullptr)
{

}

CWatchdogService::~CWatchdogService()
{
	SAFE_DELETE(this->m_pNormalProtobufFactory);
	SAFE_DELETE(this->m_pJsonProtobufFactory);
}

core::CProtobufFactory* CWatchdogService::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

core::CProtobufFactory* CWatchdogService::getForwardProtobufFactory() const
{
	return this->m_pJsonProtobufFactory;
}

CActorFactory* CWatchdogService::getActorFactory(const std::string& szType) const
{
	return this->m_pPlayerFactory;
}

void CWatchdogService::release()
{
	delete this;
}

bool CWatchdogService::onInit()
{
	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	this->m_pJsonProtobufFactory = new CJsonProtobufFactory();

	this->m_pPlayerFactory = new CPlayerFactory();
	this->m_pWatchdogServiceMessageHandler = new CWatchdogServiceMessageHandler(this);
	this->m_pPlayerMessageHandler = new CPlayerMessageHandler(this);

	this->setServiceConnectCallback(std::bind(&CWatchdogService::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));

	this->m_tickerNotifyGateOnlineCount.setCallback(std::bind(&CWatchdogService::onNotifyGateOnlineCount, this, std::placeholders::_1));
	this->registerTicker(&this->m_tickerNotifyGateOnlineCount, 5000, 5000, 0);

	PrintInfo("CWatchdogService::onInit");

	return true;
}

void CWatchdogService::onFrame()
{
	//PrintInfo("CWatchdogService::onFrame");
}

void CWatchdogService::onQuit()
{
	PrintInfo("CWatchdogService::onQuit");
}

void CWatchdogService::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "dispatch")
	{
		gas_online_count_notify msg;
		msg.set_count(0);
		this->getServiceInvoker()->send(eMTT_Service, nServiceID, &msg);
	}
}

void CWatchdogService::onNotifyGateOnlineCount(uint64_t nContext)
{
	gas_online_count_notify msg;
	msg.set_count(0);
	this->getServiceInvoker()->broadcast("dispatch", &msg);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CWatchdogService();
}