// lanucher.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <stdint.h>
#include <stdio.h>

#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include<dlfcn.h>
#endif

#include "tinyxml2/tinyxml2.h"

#include "libBaseCommon/function_util.h"
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/coroutine.h"
#include <thread>

typedef core::CServiceBase*(*funcCreateServiceBase)(const core::SServiceBaseInfo&, const std::string&);

uint64_t nCoID = 0;

void co_fun(uint64_t nContext)
{
	while (1)
	{
		printf("co_fun AAAAAA\n");
		core::coroutine::yield();
		printf("co_fun BBBBBB\n");
	}
}

void thread_fun()
{
	printf("thread_fun AAAAAA\n");
	core::coroutine::init(1024*1024);
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		printf("thread_fun BBBBBB\n");
		core::coroutine::resume(nCoID, 0);
		printf("thread_fun CCCCCC\n");
	}
}

int32_t main(int32_t argc, char* argv[])
{
// 	std::thread t = std::thread(std::bind(&thread_fun));
// 
// 	core::coroutine::init(1024 * 1024);
// 	nCoID = core::coroutine::create(1024*64, std::bind(&co_fun, std::placeholders::_1));
// 	core::coroutine::resume(nCoID, 0);
// 
// 	for (;;)
// 	{
// 		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//  }

	if (argc < 2)
	{
		fprintf(stderr, "argc error\n");
		return 0;
	}


	core::CBaseApp* pBaseApp = new core::CBaseApp();

	char szOldCurPath[MAX_PATH] = { 0 };
	_getcwd(szOldCurPath, MAX_PATH);

	// 切换到根目录
#ifdef _WIN32
	_chdir("../../../");
#else
	_chdir("../../");
#endif

	char szCurPath[MAX_PATH] = { 0 };
	_getcwd(szCurPath, MAX_PATH);

	std::string szConfig = szCurPath;
	szConfig += "/etc/";
	szConfig += argv[1];

	tinyxml2::XMLDocument sConfigXML;
	if (sConfigXML.LoadFile(szConfig.c_str()) != tinyxml2::XML_SUCCESS)
	{
		fprintf(stderr, "load etc config error\n");
		return 0;
	}

	_chdir(szOldCurPath);

	tinyxml2::XMLElement* pRootXML = sConfigXML.RootElement();
	if (pRootXML == nullptr)
	{
		fprintf(stderr, "pRootXML == nullptr\n");
		return false;
	}

	tinyxml2::XMLElement* pNodeInfoXML = pRootXML->FirstChildElement("node_info");
	if (pRootXML == nullptr)
	{
		fprintf(stderr, "pNodeInfoXML == nullptr\n");
		return false;
	}

	std::vector<void*> vecModule;
	std::vector<core::CServiceBase*> vecServiceBase;
	for (tinyxml2::XMLElement* pServiceInfoXML = pNodeInfoXML->FirstChildElement("service_info"); pServiceInfoXML != nullptr; pServiceInfoXML = pServiceInfoXML->NextSiblingElement("service_info"))
	{
		std::string szLibName = pServiceInfoXML->Attribute("lib_name");
#ifdef _WIN32
		szLibName += ".dll";
		HMODULE hModule = LoadLibraryA(szLibName.c_str());
		if (hModule == nullptr)
		{
			fprintf(stderr, "hModule == nullptr lib_name: %s error_code: %d", szLibName.c_str(), base::getLastError());
			return false;
		}

		funcCreateServiceBase pCreateServiceBase = reinterpret_cast<funcCreateServiceBase>(GetProcAddress(hModule, "createServiceBase"));
		if (nullptr == pCreateServiceBase)
		{
			fprintf(stderr, "nullptr == pCreateServiceBase lib_name: %s", szLibName.c_str());
			return false;
		}
#else
		szLibName += ".so";
		void* hModule = dlopen(szLibName.c_str(), RTLD_NOW);
		if (hModule == nullptr)
		{
			const char* szErr = dlerror();
			if (szErr != nullptr)
			{
				fprintf(stderr, "hModule == nullptr lib_name: %s error: %s", szLibName.c_str(), szErr);
				return false;
			}

			fprintf(stderr, "hModule == nullptr lib_name: %s", szLibName.c_str());
			return false;
		}

		//获取函数的地址
		funcCreateServiceBase pCreateServiceBase = reinterpret_cast<funcCreateServiceBase>(dlsym(hModule, "createServiceBase"));
		if (nullptr == pCreateServiceBase)
		{
			const char* szErr = dlerror();
			if (szErr != nullptr)
			{
				fprintf(stderr, "nullptr == pCreateServiceBase lib_name: %s error: %s", szLibName.c_str(), szErr);
				return false;
			}

			fprintf(stderr, "nullptr == pCreateServiceBase lib_name: %s", szLibName.c_str());
			return false;
		}
#endif

		core::SServiceBaseInfo sServiceBaseInfo;
		sServiceBaseInfo.nID = pServiceInfoXML->UnsignedAttribute("service_id");
		sServiceBaseInfo.szName = pServiceInfoXML->Attribute("service_name");
		sServiceBaseInfo.szType = pServiceInfoXML->Attribute("service_type");
		std::string szConfigFileName;
		if (pServiceInfoXML->Attribute("config_file_name") != nullptr)
		{
			szConfigFileName = szCurPath;
			szConfigFileName += "/etc/";
			szConfigFileName += pServiceInfoXML->Attribute("config_file_name");
		}

		core::CServiceBase* pServiceBase = pCreateServiceBase(sServiceBaseInfo, szConfigFileName);
		if (nullptr == pServiceBase)
		{
			fprintf(stderr, "create service_base error: lib_name: %s", szLibName.c_str());
			return false;
		}

		vecServiceBase.push_back(pServiceBase);
		
		vecModule.push_back(hModule);
	}

	pBaseApp->runAndServe(argc, argv, vecServiceBase);

	for (size_t i = 0; i < vecModule.size(); ++i)
	{
		void* hModule = vecModule[i];
#ifdef _WIN32
		FreeLibrary((HMODULE)hModule);
#else
		dlclose(hModule);
#endif
	}

	return 0;
}