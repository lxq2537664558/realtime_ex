// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_actor_app1.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/message_registry.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_app.h"

#include "../common/test_message_define.h"
#include "libCoreServiceKit/response_future.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/memory_hook.h"
#include "libCoreServiceKit/actor.h"

CTestActorApp1::CTestActorApp1()
{
}

CTestActorApp1::~CTestActorApp1()
{
}

CTestActorApp1* CTestActorApp1::Inst()
{
	return static_cast<CTestActorApp1*>(core::CCoreServiceApp::Inst());
}

class CTestActor1 : public core::CActor
{
public:
	CTestActor1()
	{
		m_nID = 0;
	}

	virtual ~CTestActor1()
	{

	}

	virtual void onDispatch(uint64_t nFrom, uint8_t nMessageType, core::CMessage pMessage)
	{
		if (nMessageType == eMT_REQUEST)
		{
			uint64_t nActorID = reinterpret_cast<const CServiceRequestActor1*>(pMessage.get())->nActorID;

			while (true)
			{
				CServiceRequestActor2 netMsg;
				netMsg.nID = this->m_nID++;
				core::CMessage pResultData;
				this->invoke(nActorID, &netMsg, pResultData);
				PrintInfo("Actor1 ID: %d", reinterpret_cast<const CServiceResponseActor2*>(pResultData.get())->nID);
			}
		}
	}

private:
	uint32_t	m_nID;
};

class CTestActor2 : public core::CActor
{
public:
	CTestActor2()
	{

	}

	virtual ~CTestActor2()
	{

	}

	virtual void onDispatch(uint64_t nFrom, uint8_t nMessageType, core::CMessage pMessage)
	{
		if (nMessageType == eMT_REQUEST)
		{
			CServiceResponseActor2 netMsg;
			netMsg.nID = reinterpret_cast<const CServiceRequestActor2*>(pMessage.get())->nID;

			this->response(&netMsg);
		}
	}

	virtual void		onForward(core::SClientSessionInfo sClientSessionInfo, uint8_t nMessageType, core::CMessage pMessage) { }
};

bool CTestActorApp1::onInit()
{
	CCoreServiceApp::onInit();

	core::CActor* pActor1 = new CTestActor1();
	core::CActor* pActor2 = new CTestActor2();

	for (size_t i = 0; i < 100; ++i)
	{
		CServiceRequestActor1 netMsg;
		netMsg.nActorID = pActor2->getID();
		pActor1->invoke(pActor1->getID(), &netMsg);
	}
	return true;
}

void CTestActorApp1::onDestroy()
{
	CCoreServiceApp::onDestroy();
}

void CTestActorApp1::onQuit()
{
	this->doQuit();
}

int32_t main(int argc, char* argv[])
{
	CTestActorApp1* pTestActorApp1 = new CTestActorApp1();
	pTestActorApp1->run(argc, argv, "test_actor_config1.xml");

	return 0;
}