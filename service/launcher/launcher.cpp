// container.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "libCoreCommon/base_app.h"

#include "../../test/test_service1/test_service1.h"
#include "../../test/test_service2/test_service2.h"
#include "../../test/test_service3/test_service3.h"
#include "../../test/test_actor1/test_actor_service1.h"
#include "../../test/test_actor2/test_actor_service2.h"

int32_t main(int32_t argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "argc error\n");
		return 0;
	}

	CTestService1::registerClassInfo();
	CTestService2::registerClassInfo();
	CTestService3::registerClassInfo();

	CTestActorService1::registerClassInfo();
	CTestActorService2::registerClassInfo();

	core::CBaseApp* pBaseApp = new core::CBaseApp();
	pBaseApp->run(argv[0], argv[1]);
	SAFE_DELETE(pBaseApp);

	return 0;
}