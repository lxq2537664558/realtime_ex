// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_actor_app2.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_app.h"

#include "../common/test_message_define.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/memory_hook.h"
#include "libCoreServiceKit/base_actor.h"
#include "libCoreServiceKit/base_actor_factory.h"
#include "libCoreServiceKit/actor_message_registry.h"

CTestActorApp2::CTestActorApp2()
{
}

CTestActorApp2::~CTestActorApp2()
{
}

CTestActorApp2* CTestActorApp2::Inst()
{
	return static_cast<CTestActorApp2*>(core::CCoreServiceApp::Inst());
}

class CTestActor1
	: public core::CBaseActor
	, public core::CActorMessageRegistry<CTestActor1>
{
public:
	CTestActor1()
	{

	}

	virtual ~CTestActor1()
	{

	}

	DEFEND_ACTOR_MESSAGE_FUNCTION(CTestActor1)

	virtual bool onInit(void* pContext)
	{
		REGISTER_ACTOR_MESSAGE_HANDLER(CTestActor1, eServiceRequestActor4, &CTestActor1::onRequest3, true);
		return true;
	}

	void onRequest3(CBaseActor* pBaseActor, uint64_t nFrom, std::shared_ptr<CServiceRequestActor3> pMessage)
	{
		CServiceResponseActor4 netMsg;
		netMsg.nID = reinterpret_cast<const CServiceRequestActor4*>(pMessage.get())->nID;

		this->response(&netMsg);
	}

	void onRequest4(CBaseActor* pBaseActor, uint64_t nFrom, std::shared_ptr<CServiceRequestActor4> pMessage)
	{
		CServiceResponseActor4 netMsg;
		netMsg.nID = pMessage->nID;

		this->response(&netMsg);
	}
};

class CTestActorFactory1 :
	public core::CBaseActorFactory
{
public:
	core::CBaseActor* createBaseActor()
	{
		return new CTestActor1();
	}
};

bool CTestActorApp2::onInit()
{
	CCoreServiceApp::onInit();

	CTestActorFactory1* pTestActorFactory1 = new CTestActorFactory1();
	
	core::CBaseActor* pActor1 = core::CBaseActor::createActor("", pTestActorFactory1);
	
	return true;
}

void CTestActorApp2::onDestroy()
{
	CCoreServiceApp::onDestroy();
}

void CTestActorApp2::onQuit()
{
	this->doQuit();
}

int32_t main(int argc, char* argv[])
{
	CTestActorApp2* pTestActorApp2 = new CTestActorApp2();
	pTestActorApp2->run(argc, argv, "test_actor_config2.xml");

	return 0;
}