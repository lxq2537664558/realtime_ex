// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app1.h"

#include "libCoreCommon/message_registry.h"

void client_request_msg_callback(const core::SClientSessionInfo& sClientSessionInfo, uint32_t nMessageType, const google::protobuf::Message* pMessage)
{

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
	core::CMessageRegistry::Inst()->registerGateForwardCallback("test.client_request_msg", &client_request_msg_callback);

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
	pTestServiceApp1->run(true, argc, argv, "test_service_config1.xml");

	return 0;
}