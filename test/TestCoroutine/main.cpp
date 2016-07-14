#include "libBaseCommon/base_time.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/coroutine.h"


class CTestApp;
CTestApp* g_pApp;

void fun1(uint64_t nContext)
{
	PrintDebug("AAAAA");
 	core::coroutine::yield();
 	PrintDebug("BBBBB");
}

#include <iostream>
#include <queue>
using namespace std;

class CTestApp :
	public core::CBaseApp
{
public:
	virtual bool onInit()
	{	
		uint64_t nCoroutineID = core::coroutine::start(std::bind(&fun1, std::placeholders::_1));
		core::coroutine::resume(nCoroutineID, 0);
		core::coroutine::resume(nCoroutineID, 0);

		nCoroutineID = core::coroutine::start(std::bind(&fun1, std::placeholders::_1));
		core::coroutine::resume(nCoroutineID, 0);
		core::coroutine::resume(nCoroutineID, 0);

		return true;
	}

	virtual void onQuit()
	{
		
	}
};

int32_t main(int32_t argc, char* argv[])
{
	g_pApp = new CTestApp();
	g_pApp->run(argc, argv, "gate_config.xml");

	return 0;
}