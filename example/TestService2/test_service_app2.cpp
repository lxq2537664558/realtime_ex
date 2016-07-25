// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app2.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/message_registry.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_kit.h"

#include "../common/test_message_define.h"

void service_request_msg_callback1(const std::string szFromService, uint32_t nMessageType, core::message_header_ptr pMessage)
{
	SServiceResponseMsg1 service_msg;
	service_msg.nID = std::static_pointer_cast<const SServiceRequestMsg1>(pMessage)->nID;
	service_msg.nClientTime = std::static_pointer_cast<const SServiceRequestMsg1>(pMessage)->nClientTime;

	core::CClusterInvoker::Inst()->response(&service_msg);
}

void service_request_msg_callback2(const std::string szFromService, uint32_t nMessageType, core::message_header_ptr pMessage)
{
	SServiceResponseMsg2 service_msg;
	service_msg.nID = std::static_pointer_cast<const SServiceRequestMsg2>(pMessage)->nID;
	service_msg.nClientTime = std::static_pointer_cast<const SServiceRequestMsg2>(pMessage)->nClientTime;

	core::CClusterInvoker::Inst()->response(&service_msg);
}

void service_request_msg_callback3(const std::string szFromService, uint32_t nMessageType, core::message_header_ptr pMessage)
{
	SServiceResponseMsg3 service_msg;
	service_msg.nID = std::static_pointer_cast<const SServiceRequestMsg3>(pMessage)->nID;
	service_msg.nClientTime = std::static_pointer_cast<const SServiceRequestMsg3>(pMessage)->nClientTime;

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
	core::CMessageRegistry::Inst()->registerServiceCallback(eServiceRequestMsg1, &service_request_msg_callback1);
	core::CMessageRegistry::Inst()->registerServiceCallback(eServiceRequestMsg2, &service_request_msg_callback2);
	core::CMessageRegistry::Inst()->registerServiceCallback(eServiceRequestMsg3, &service_request_msg_callback3);

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