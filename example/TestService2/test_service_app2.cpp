// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app2.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_app.h"

#include "../common/test_message_define.h"

void service_request_msg_callback1(uint16_t nFromServiceID, core::CMessage pMessage)
{
	SServiceResponseMsg1 service_msg;
	service_msg.nClientID = reinterpret_cast<const SServiceRequestMsg1*>(pMessage.get())->nClientID;
	service_msg.nID = reinterpret_cast<const SServiceRequestMsg1*>(pMessage.get())->nID;
	service_msg.nClientTime = reinterpret_cast<const SServiceRequestMsg1*>(pMessage.get())->nClientTime;

	core::cluster_invoker::response(&service_msg);
}

void service_request_msg_callback2(uint16_t nFromServiceID, core::CMessage pMessage)
{
	SServiceResponseMsg2 service_msg;
	service_msg.nClientID = reinterpret_cast<const SServiceRequestMsg2*>(pMessage.get())->nClientID;
	service_msg.nID = reinterpret_cast<const SServiceRequestMsg2*>(pMessage.get())->nID;
	service_msg.nClientTime = reinterpret_cast<const SServiceRequestMsg2*>(pMessage.get())->nClientTime;

	core::cluster_invoker::response(&service_msg);
}

void service_request_msg_callback3(uint16_t nFromServiceID, core::CMessage pMessage)
{
	SServiceResponseMsg3 service_msg;
	service_msg.nClientID = reinterpret_cast<const SServiceRequestMsg3*>(pMessage.get())->nClientID;
	service_msg.nID = reinterpret_cast<const SServiceRequestMsg3*>(pMessage.get())->nID;
	service_msg.nClientTime = reinterpret_cast<const SServiceRequestMsg3*>(pMessage.get())->nClientTime;

	core::cluster_invoker::response(&service_msg);
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
	CCoreServiceApp::onInit();
	this->registerServiceCallback(eServiceRequestMsg1, &service_request_msg_callback1);
	this->registerServiceCallback(eServiceRequestMsg2, &service_request_msg_callback2);
	this->registerServiceCallback(eServiceRequestMsg3, &service_request_msg_callback3);

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