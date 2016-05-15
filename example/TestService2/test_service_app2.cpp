// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app2.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/message_registry.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/load_balance.h"
#include "libCoreServiceKit/core_service_kit.h"

#include "../proto_src/service_request_msg.pb.h"
#include "../proto_src/client_request_msg.pb.h"
#include "../proto_src/service_response_msg.pb.h"
#include "../proto_src/client_response_msg.pb.h"

void service_request_msg_callback(const std::string& szFromService, uint32_t nMessageType, const google::protobuf::Message* pMessage)
{
	const test::service_request_msg* pRequestMsg = dynamic_cast<const test::service_request_msg*>(pMessage);
	DebugAst(pRequestMsg != nullptr);

	test::service_response_msg msg;
	msg.set_name(pRequestMsg->name());
	msg.set_id(pRequestMsg->id());

	core::CClusterInvoker::Inst()->response(&msg);
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
	pTestServiceApp1->run(argc, argv, "test_service_config2.xml");

	return 0;
}