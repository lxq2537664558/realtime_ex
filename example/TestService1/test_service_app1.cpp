// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app1.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/message_registry.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_kit.h"

#include "../common/test_message_define.h"

void client_request_msg_callback(const core::SClientSessionInfo sClientSessionInfo, uint32_t nMessageType, core::message_header_ptr pMessage)
{
	SServiceRequestMsg service_msg;
	service_msg.nID = std::static_pointer_cast<const SClientRequestMsg>(pMessage)->nID;

	core::message_header_ptr pResultData = nullptr;
	uint32_t nRet = core::CClusterInvoker::Inst()->invok("test2-1", &service_msg, pResultData);
	if (nRet != eRRT_OK)
	{
		PrintDebug("AAAAAAAA");
		return;
	}

	SClientResponseMsg client_msg;
	client_msg.nID = std::static_pointer_cast<const SClientRequestMsg>(pResultData)->nID;
	core::CClusterInvoker::Inst()->send(sClientSessionInfo, &client_msg);
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

bool CTestServiceApp1::onInit()
{
	core::CCoreServiceKit::Inst()->init();
	core::CMessageRegistry::Inst()->registerGateForwardCallback(eClientRequestMsg, &client_request_msg_callback);
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