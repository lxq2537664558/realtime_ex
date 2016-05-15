// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_app1.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/message_registry.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/load_balance.h"

#include "../proto_src/service_request_msg.pb.h"
#include "../proto_src/client_request_msg.pb.h"
#include "../proto_src/service_response_msg.pb.h"
#include "../proto_src/client_response_msg.pb.h"
#include "libCoreServiceKit/core_service_kit.h"

void client_request_msg_callback(const core::SClientSessionInfo& sClientSessionInfo, uint32_t nMessageType, const google::protobuf::Message* pMessage)
{
	const test::client_request_msg* pClientMsg = dynamic_cast<const test::client_request_msg*>(pMessage);
	DebugAst(pClientMsg != nullptr);

	test::service_request_msg msg;
	msg.set_name(pClientMsg->name());
	msg.set_id(pClientMsg->id());

	core::CClusterInvoker::Inst()->invok_r(&msg, 0, "*", [sClientSessionInfo](uint32_t nMessageType, const google::protobuf::Message* pMessage, core::EResponseResultType eType)->void
	{
		const test::service_response_msg* pResponseMsg = dynamic_cast<const test::service_response_msg*>(pMessage);
		DebugAst(pResponseMsg != nullptr);
		test::client_response_msg msg;
		msg.set_name(pResponseMsg->name());
		msg.set_id(pResponseMsg->id());
		core::CClusterInvoker::Inst()->send(sClientSessionInfo, &msg);
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
	return static_cast<CTestServiceApp1*>(core::CBaseApp::Inst());
}

bool CTestServiceApp1::onInit()
{
	core::CCoreServiceKit::Inst()->init();
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
	pTestServiceApp1->run(argc, argv, "test_service_config1.xml");

	return 0;
}