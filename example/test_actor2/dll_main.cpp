#include <Windows.h>

#include "test_actor_service2.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		CTestActorService2::registerClassInfo();
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