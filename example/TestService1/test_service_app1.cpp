// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app1.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/message_registry.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_kit.h"

#include "../common/test_message_define.h"
#include "../src/libCoreCommon/memory_hook.h"
#include "libCoreServiceKit/response_promise.h"

void client_request_msg_callback(const core::SClientSessionInfo sClientSessionInfo, uint32_t nMessageType, core::message_header_ptr pMessage)
{
	uint32_t nSync = std::static_pointer_cast<const SClientRequestMsg>(pMessage)->nSync;
	if (nSync != 0)
	{
		SServiceRequestMsg1 service_msg1;
		service_msg1.nID = std::static_pointer_cast<const SClientRequestMsg>(pMessage)->nID;
		service_msg1.nClientTime = std::static_pointer_cast<const SClientRequestMsg>(pMessage)->nClientTime;
		core::message_header_ptr pResultData1 = nullptr;
		uint32_t nRet = core::CClusterInvoker::Inst()->invok("test2-1", &service_msg1, pResultData1);
		if (nRet != eRRT_OK)
		{
			PrintDebug("AAAAAAAA");
			return;
		}

		SServiceRequestMsg2 service_msg2;
		service_msg2.nID = std::static_pointer_cast<const SServiceResponseMsg1>(pResultData1)->nID;
		service_msg2.nClientTime = std::static_pointer_cast<const SServiceResponseMsg1>(pResultData1)->nClientTime;
		core::message_header_ptr pResultData2 = nullptr;
		nRet = core::CClusterInvoker::Inst()->invok("test2-1", &service_msg2, pResultData2);
		if (nRet != eRRT_OK)
		{
			PrintDebug("BBBBBBBBB");
			return;
		}

		SServiceRequestMsg3 service_msg3;
		service_msg3.nID = std::static_pointer_cast<const SServiceResponseMsg2>(pResultData2)->nID;
		service_msg3.nClientTime = std::static_pointer_cast<const SServiceResponseMsg2>(pResultData2)->nClientTime;
		core::message_header_ptr pResultData3 = nullptr;
		nRet = core::CClusterInvoker::Inst()->invok("test2-1", &service_msg3, pResultData3);
		if (nRet != eRRT_OK)
		{
			PrintDebug("CCCCCCCC");
			return;
		}

		SClientResponseMsg client_msg;
		client_msg.nID = std::static_pointer_cast<const SServiceRequestMsg3>(pResultData3)->nID;
		client_msg.nClientTime = std::static_pointer_cast<const SServiceRequestMsg3>(pResultData3)->nClientTime;
		core::CClusterInvoker::Inst()->send(sClientSessionInfo, &client_msg);
	}
	else
	{
		SServiceRequestMsg1 service_msg1;
		service_msg1.nID = std::static_pointer_cast<const SClientRequestMsg>(pMessage)->nID;
		service_msg1.nClientTime = std::static_pointer_cast<const SClientRequestMsg>(pMessage)->nClientTime;
		core::CResponsePromise sResponsePromise;
		bool bRet = core::CClusterInvoker::Inst()->invok_r("test2-1", &service_msg1, sResponsePromise);
		if (!bRet)
		{
			PrintDebug("AAAAAAAA");
			return;
		}

		sResponsePromise.then_r([sClientSessionInfo, nSync](uint8_t, core::message_header_ptr pResultData)
		{
			SServiceRequestMsg2 service_msg2;
			service_msg2.nID = std::static_pointer_cast<const SServiceResponseMsg1>(pResultData)->nID;
			service_msg2.nClientTime = std::static_pointer_cast<const SServiceResponseMsg1>(pResultData)->nClientTime;

			core::CResponsePromise sResponsePromise;
			bool bRet = core::CClusterInvoker::Inst()->invok_r("test2-1", &service_msg2, sResponsePromise);
			if (!bRet)
				PrintDebug("BBBBBBBBB");
		
			return sResponsePromise;

		}).then_r([sClientSessionInfo, nSync](uint8_t, core::message_header_ptr pResultData)
		{
			SServiceRequestMsg3 service_msg3;
			service_msg3.nID = std::static_pointer_cast<const SServiceResponseMsg2>(pResultData)->nID;
			service_msg3.nClientTime = std::static_pointer_cast<const SServiceResponseMsg2>(pResultData)->nClientTime;

			core::CResponsePromise sResponsePromise;
			bool bRet = core::CClusterInvoker::Inst()->invok_r("test2-1", &service_msg3, sResponsePromise);
			if (!bRet)
				PrintDebug("CCCCCCCC");

			return sResponsePromise;

		}).then([sClientSessionInfo, nSync](uint8_t, core::message_header_ptr pResultData)
		{
			SClientResponseMsg client_msg;
			client_msg.nID = std::static_pointer_cast<const SServiceRequestMsg3>(pResultData)->nID;
			client_msg.nClientTime = std::static_pointer_cast<const SServiceRequestMsg3>(pResultData)->nClientTime;
			client_msg.nSync = nSync;
			core::CClusterInvoker::Inst()->send(sClientSessionInfo, &client_msg);
		});
	}
}

CTestServiceApp1::CTestServiceApp1()
{
}

CTestServiceApp1::~CTestServiceApp1()
{
}

CTestServiceApp1* CTestServiceApp1::Inst()
{
	return static_cast<CTestServiceApp1*>(core::CBaseApp::Inst());
}

void tick_fun1(uint64_t nContext)
{
	PrintInfo("AAAAAA");
	core::beginMemoryLeakChecker(false);
}

void tick_fun2(uint64_t nContext)
{
	PrintInfo("BBBBBB");
	core::endMemoryLeakChecker("memory.txt");
}

bool CTestServiceApp1::onInit()
{
	core::CCoreServiceKit::Inst()->init();
	core::CMessageRegistry::Inst()->registerGateForwardCallback(eClientRequestMsg, &client_request_msg_callback);
	core::CTicker* pTicker = new core::CTicker();
	pTicker->setCallback(std::bind(&tick_fun1, std::placeholders::_1));
	this->registerTicker(pTicker, 10000, 0, 0);
	pTicker = new core::CTicker();
	pTicker->setCallback(std::bind(&tick_fun2, std::placeholders::_1));
	this->registerTicker(pTicker, 700000, 0, 0);

	return true;
}

void CTestServiceApp1::onDestroy()
{
}

void CTestServiceApp1::onQuit()
{
	this->doQuit();
}

int32_t main(int argc, char* argv[])
{
	CTestServiceApp1* pTestServiceApp1 = new CTestServiceApp1();
	pTestServiceApp1->run(argc, argv, "test_service_config1.xml");

	return 0;
}