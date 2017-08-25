// container.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <stdint.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include<dlfcn.h>
#endif

typedef void(*funcRunAndServe)(const std::string&, const std::string&);

int32_t main(int32_t argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "argc error\n");
		return 0;
	}

	funcRunAndServe pRunAndServe = nullptr;
#ifdef _WIN32
	HMODULE hModule = LoadLibraryA("libCoreCommon.dll");
	if (hModule == nullptr)
	{
		fprintf(stderr, "load libCoreCommon.so error %d\n", GetLastError());
		return false;
	}

	pRunAndServe = reinterpret_cast<funcRunAndServe>(GetProcAddress(hModule, "runAndServe"));
	if (nullptr == pRunAndServe)
	{
		fprintf(stderr, "load libCoreCommon.so nullptr == pRunAndServe error %d\n", GetLastError());
		return false;
	}
#else
	void* hModule = dlopen("libCoreCommon.so", RTLD_NOW);
	if (hModule == nullptr)
	{
		const char* szErr = dlerror();
		if (szErr != nullptr)
		{
			fprintf(stderr, "load libCoreCommon.so error %s\n", szErr);
			return 0;
		}

		fprintf(stderr, "load libCoreCommon.so error\n");
		return 0;
	}

	//获取函数的地址
	pRunAndServe = reinterpret_cast<funcRunAndServe>(dlsym(hModule, "runAndServe"));
	if (nullptr == pRunAndServe)
	{
		const char* szErr = dlerror();
		if (szErr != nullptr)
		{
			fprintf(stderr, "load libCoreCommon.so nullptr == pRunAndServe error: %s", szErr);
			return false;
		}

		fprintf(stderr, "load libCoreCommon.so nullptr == pRunAndServe error");
		return false;
	}
#endif

	pRunAndServe(argv[0], argv[1]);

#ifdef _WIN32
	FreeLibrary(hModule);
#else
	dlclose(hModule);
#endif

	return 0;
}