// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app2.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/message_registry.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_kit.h"

#include "../common/test_message_define.h"

void service_request_msg_callback(const std::string szFromService, uint32_t nMessageType, core::message_header_ptr pMessage)
{
	SServiceResponseMsg service_msg;
	service_msg.nID = std::static_pointer_cast<const SServiceRequestMsg>(pMessage)->nID;

	core::CClusterInvoker::Inst()->response(&service_msg);
}

CTestServiceApp2::CTestServiceApp2()
{
}

CTestServiceApp2::~CTestServiceApp2()
{
}

CTestServiceApp2* CTestServiceApp2::Inst()
{
	return static_cast<CTestServiceApp2*>(core::CBaseApp::Inst());
}

bool CTestServiceApp2::onInit()
{
	core::CCoreServiceKit::Inst()->init();
	core::CMessageRegistry::Inst()->registerServiceCallback(eServiceRequestMsg, &service_request_msg_callback);

	return true;
}

void CTestServiceApp2::onDestroy()
{
}

void CTestServiceApp2::onQuit()
{
	this->doQuit();
}

int32_t main(int argc, char* argv[])
{
	CTestServiceApp2* pTestServiceApp1 = new CTestServiceApp2();
	pTestServiceApp1->run(argc, argv, "test_service_config2.xml");

	return 0;
}