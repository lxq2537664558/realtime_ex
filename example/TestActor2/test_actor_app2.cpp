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
#include "libCoreServiceKit/actor.h"

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

class CTestActor1 : public core::CActor
{
public:
	CTestActor1()
	{

	}

	virtual ~CTestActor1()
	{

	}

	virtual void onDispatch(uint64_t nFrom, uint8_t nMessageType, core::CMessage pMessage)
	{
		if (nMessageType == eMT_REQUEST)
		{
			CServiceResponseActor4 netMsg;
			netMsg.nID = reinterpret_cast<const CServiceRequestActor4*>(pMessage.get())->nID;

			this->response(&netMsg);
		}
	}

	virtual void		onForward(core::SClientSessionInfo sClientSessionInfo, uint8_t nMessageType, core::CMessage pMessage) { }
};

bool CTestActorApp2::onInit()
{
	CCoreServiceApp::onInit();

	core::CActor* pActor1 = new CTestActor1();
	
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