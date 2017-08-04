#include "watchdog_service.h"
#include "watchdog_service_message_handler.h"
#include "player_message_handler.h"
#include "player.h"

CWatchdogService::CWatchdogService()
	: m_pWatchdogServiceMessageHandler(nullptr)
	, m_pPlayerMessageHandler(nullptr)
	, m_pDefaultProtobufFactory(nullptr)
	, m_pPlayerFactory(nullptr)
{

}

CWatchdogService::~CWatchdogService()
{

}

core::CProtobufFactory* CWatchdogService::getProtobufFactory() const
{
	return this->m_pDefaultProtobufFactory;
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
	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	this->m_pPlayerFactory = new CPlayerFactory();
	this->m_pWatchdogServiceMessageHandler = new CWatchdogServiceMessageHandler(this);
	this->m_pPlayerMessageHandler = new CPlayerMessageHandler(this);

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

extern "C" __declspec(dllexport) CServiceBase* createServiceBase()
{
	return new CWatchdogService();
}