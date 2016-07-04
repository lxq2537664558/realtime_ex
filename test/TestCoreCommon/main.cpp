#include "libBaseCommon/base_time.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/coroutine.h"

#include <map>

#include <stdio.h>
#include "libBaseCommon/rand_gen.h"

class CTestApp;
CTestApp* g_pApp;

void fun1(uint64_t nContext)
{
	while (true)
	{
		core::coroutine::sleep(10000);
		PrintDebug("111111111 %d", (uint32_t)nContext);
	}
}

void fun2(uint64_t nContext)
{

}

class CTestApp :
	public core::CBaseApp
{
public:
	virtual bool onInit()
	{
// 		uint64_t nID = core::coroutine::start(std::bind(&fun1, std::placeholders::_1));
// 		
// 		core::coroutine::resume(nID, 100);

		for (size_t i = 0; i < 10000000; ++i)
		{
			core::CTicker* pTicker = new core::CTicker();
			pTicker->setCallback(std::bind(&fun2, std::placeholders::_1));

			int64_t nTime = base::CRandGen::getGlobalRand(1000000000);
			this->registerTicker(pTicker, nTime, nTime, 0);
		}
// 		nID = core::coroutine::start(std::bind(&fun1, std::placeholders::_1));
// 
// 		core::coroutine::resume(nID, 200);
// 
// 		nID = core::coroutine::start(std::bind(&fun1, std::placeholders::_1));
// 
// 		core::coroutine::resume(nID, 300);
		this->getBaseConnectionMgr()->listen("127.0.0.1", 8888, 0, "", 1024, 1024);
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