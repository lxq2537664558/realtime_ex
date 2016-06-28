#include "libBaseCommon/base_time.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/coroutine.h"

#include <map>


class CTestApp;
CTestApp* g_pApp;

void fun1(uint64_t nContext)
{
	while (true)
	{
		nContext = core::coroutine::yield();
		//PrintDebug("111111111 %d", (uint32_t)nContext);
	}
}

class CTestApp :
	public core::CBaseApp
{
public:
	virtual bool onInit()
	{
		uint64_t nID = core::coroutine::startCoroutine(std::bind(&fun1, std::placeholders::_1));
		for (uint32_t i = 0; i < 1000000000; ++i)
		{
			//PrintDebug("222222");
			core::coroutine::resume(nID, i);
		}
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