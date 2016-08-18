// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app1.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_app.h"

#include "../common/test_message_define.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/memory_hook.h"

void client_request_msg_callback(const core::SClientSessionInfo sClientSessionInfo, core::CMessage pMessage)
{
	uint32_t nSync = reinterpret_cast<const SClientRequestMsg*>(pMessage.get())->nSync;
	
	SServiceRequestMsg1 service_msg1;
	service_msg1.nClientID = reinterpret_cast<const SClientRequestMsg*>(pMessage.get())->nClientID;
	service_msg1.nID = reinterpret_cast<const SClientRequestMsg*>(pMessage.get())->nID;
	service_msg1.nClientTime = reinterpret_cast<const SClientRequestMsg*>(pMessage.get())->nClientTime;
	core::CFuture<std::shared_ptr<SServiceResponseMsg1>> sResponseFuture;
	bool bRet = core::CClusterInvoker::Inst()->invoke_r("test2-1", &service_msg1, sResponseFuture);
	if (!bRet)
	{
		PrintDebug("AAAAAAAA");
		return;
	}

// 	if (service_msg1.nClientID == 1)
// 		PrintInfo("time1: "UINT64FMT, base::getProcessPassTime());

	sResponseFuture.then_r([sClientSessionInfo, nSync](std::shared_ptr<SServiceResponseMsg1> pResultData, uint32_t nErrorCode)
	{
		SServiceRequestMsg2 service_msg2;
		service_msg2.nClientID = pResultData->nClientID;
		service_msg2.nID = pResultData->nID;
		service_msg2.nClientTime = pResultData->nClientTime;

// 		if (service_msg2.nClientID == 1)
// 			PrintInfo("time2: "UINT64FMT, base::getProcessPassTime());
		core::CFuture<std::shared_ptr<SServiceResponseMsg2>> sResponseFuture;
		bool bRet = core::CClusterInvoker::Inst()->invoke_r("test2-1", &service_msg2, sResponseFuture);
		if (!bRet)
			PrintDebug("BBBBBBBBB");

		return sResponseFuture;

	}).then_r([sClientSessionInfo, nSync](std::shared_ptr<SServiceResponseMsg2> pResultData, uint32_t nErrorCode)
	{
		SServiceRequestMsg3 service_msg3;
		service_msg3.nClientID = pResultData->nClientID;
		service_msg3.nID = pResultData->nID;
		service_msg3.nClientTime = pResultData->nClientTime;
		
// 		if (service_msg3.nClientID == 1)
// 			PrintInfo("time3: "UINT64FMT, base::getProcessPassTime());
		core::CFuture<std::shared_ptr<SServiceResponseMsg3>> sResponseFuture;
		bool bRet = core::CClusterInvoker::Inst()->invoke_r("test2-1", &service_msg3, sResponseFuture);
		if (!bRet)
			PrintDebug("CCCCCCCC");

		return sResponseFuture;

	}).then([sClientSessionInfo, nSync](std::shared_ptr<SServiceResponseMsg3> pResultData, uint32_t nErrorCode)
	{
		SClientResponseMsg client_msg;
		client_msg.nClientID = pResultData->nClientID;
		client_msg.nID = pResultData->nID;
		client_msg.nClientTime = pResultData->nClientTime;
		client_msg.nSync = nSync;
		core::CClusterInvoker::Inst()->send(sClientSessionInfo, &client_msg);
// 		if (client_msg.nClientID == 1)
// 			PrintInfo("time4: "UINT64FMT, base::getProcessPassTime());
	});
}

CTestServiceApp1::CTestServiceApp1()
{
}

CTestServiceApp1::~CTestServiceApp1()
{
}

CTestServiceApp1* CTestServiceApp1::Inst()
{
	return static_cast<CTestServiceApp1*>(core::CCoreServiceApp::Inst());
}

void tick_fun1(uint64_t nContext)
{
	PrintInfo("AAAAAA");
	base::beginMemoryLeakChecker(false);
}

void tick_fun2(uint64_t nContext)
{
	PrintInfo("BBBBBB");
	base::endMemoryLeakChecker("memory.txt");
}

void tick_fun3(uint64_t nContext)
{
	PrintInfo("memory size: "INT64FMT " QPS: %d", base::getMemorySize(), core::CBaseApp::Inst()->getQPS());
}

bool CTestServiceApp1::onInit()
{
	CCoreServiceApp::onInit();

	this->registerGateForwardCallback(eClientRequestMsg, &client_request_msg_callback);
	core::CTicker* pTicker = new core::CTicker();
	pTicker->setCallback(std::bind(&tick_fun1, std::placeholders::_1));
	this->registerTicker(pTicker, 10000, 0, 0);
	pTicker = new core::CTicker();
	pTicker->setCallback(std::bind(&tick_fun2, std::placeholders::_1));
	this->registerTicker(pTicker, 30000, 0, 0);

	pTicker = new core::CTicker();
	pTicker->setCallback(std::bind(&tick_fun3, std::placeholders::_1));
	this->registerTicker(pTicker, 3000, 3000, 0);

	return true;
}

void CTestServiceApp1::onDestroy()
{
	CCoreServiceApp::onDestroy();
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