// container.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "libCoreCommon/base_app.h"

int32_t main(int32_t argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "argc error\n");
		return 0;
	}

	core::CBaseApp* pBaseApp = new core::CBaseApp();
	pBaseApp->run(argv[0], argv[1]);
	SAFE_DELETE(pBaseApp);

	return 0;
}