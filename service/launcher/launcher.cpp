// container.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "libCoreCommon/base_app.h"
using namespace core;

int32_t main(int32_t argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "argc error\n");
		return 0;
	}

	CBaseApp* pBaseApp = new CBaseApp();
	pBaseApp->run(argv[0], argv[1]);
	SAFE_DELETE(pBaseApp);

	return 0;
}