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
	uint64_t nCoroutineID = core::coroutine::create([](uint64_t) {});
	core::coroutine::resume(nCoroutineID, 0);

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
// 		std::vector<uint64_t> vecID;
// 		for (size_t i = 0; i < 1000; ++i)
// 		{
// 			vecID.push_back(core::coroutine::create(std::bind(&fun1, std::placeholders::_1)));
// 		}
// 
// 		for (size_t i = 0; i < vecID.size(); ++i)
// 		{
// 			core::coroutine::resume(vecID[i], 0);
// 			core::coroutine::resume(vecID[i], 0);
// 		}
		uint64_t nCoroutineID = core::coroutine::create(std::bind(&fun1, std::placeholders::_1));
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