// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app2.h"

#include "libCoreCommon/message_registry.h"

void service_request_msg_callback(const std::string& szFromService, uint32_t nMessageType, const google::protobuf::Message* pMessage)
{

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
	core::CMessageRegistry::Inst()->registerServiceCallback("test.service_request_msg", &service_request_msg_callback);

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
	pTestServiceApp1->run(true, argc, argv, "test_service_config2.xml");

	return 0;
}