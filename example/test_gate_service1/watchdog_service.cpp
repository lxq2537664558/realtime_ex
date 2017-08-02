#include "watchdog_service.h"
#include "watchdog_service_message_handler.h"
#include "player_message_handler.h"
#include "player.h"

DEFINE_OBJECT(CWatchdogService, 1);

CWatchdogService::CWatchdogService()
	: m_pWatchdogServiceMessageHandler(nullptr)
	, m_pPlayerMessageHandler(nullptr)
	, m_pDefaultProtobufFactory(nullptr)
{

}

CWatchdogService::~CWatchdogService()
{

}

bool CWatchdogService::onInit()
{
	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	this->setProtobufFactory(this->m_pDefaultProtobufFactory);

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

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CWatchdogService::registerClassInfo();
		CPlayer::registerClassInfo();
		break;

	case DLL_THREAD_ATTACH:

		break;

	case DLL_THREAD_DETACH:

		break;

	case DLL_PROCESS_DETACH:

		break;
	}

	return TRUE;
}
#endif