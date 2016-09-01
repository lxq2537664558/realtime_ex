// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_actor_app3.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_app.h"

#include "../common/test_message_define.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/memory_hook.h"
#include "libCoreServiceKit/base_actor.h"
#include "libCoreServiceKit/base_actor_factory.h"

CTestActorApp3::CTestActorApp3()
{
}

CTestActorApp3::~CTestActorApp3()
{
}

CTestActorApp3* CTestActorApp3::Inst()
{
	return static_cast<CTestActorApp3*>(core::CCoreServiceApp::Inst());
}

class CTestActor1 
	: public core::CBaseActor
{
public:
	CTestActor1()
	{
		m_nID = 0;
	}

	virtual ~CTestActor1()
	{

	}

	virtual bool onInit(void* pContext)
	{
		this->registerMessageHandler(eServiceRequestActor3, std::bind(&CTestActor1::onRequest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), false);
		return true;
	}

	void onRequest(CBaseActor* pBaseActor, uint64_t nFrom, core::CMessagePtr<char> pMessage)
	{
		uint64_t nActorID = core::CMessagePtr<CServiceRequestActor3>::reinterpret_cast_message(pMessage)->nActorID;

		while (true)
		{
			CServiceRequestActor4 netMsg;
			netMsg.nID = this->m_nID++;
			core::CMessage pResultData;
			this->invoke(nActorID, &netMsg, pResultData);
			PrintInfo("Actor1 ID: %d", reinterpret_cast<const CServiceResponseActor4*>(pResultData.get())->nID);
		}
	}

private:
	uint32_t	m_nID;
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

void onNodeConnect(uint16_t nServiceID)
{
	if (nServiceID != 7)
		return;

	CTestActorFactory1* pTestActorFactory1 = new CTestActorFactory1();

	core::CBaseActor* pActor1 = core::CBaseActor::createActor("", pTestActorFactory1);

	for (size_t i = 0; i < 1; ++i)
	{
		CServiceRequestActor3 netMsg;
		netMsg.nActorID = 7ULL << 48 | 1;
		pActor1->invoke(pActor1->getID(), &netMsg);
	}
}

bool CTestActorApp3::onInit()
{
	CCoreServiceApp::onInit();

	this->setNodeConnectCallback(std::bind(&onNodeConnect, std::placeholders::_1));
	
	return true;
}


void CTestActorApp3::onDestroy()
{
	CCoreServiceApp::onDestroy();
}

void CTestActorApp3::onQuit()
{
	this->doQuit();
}

int32_t main(int argc, char* argv[])
{
	CTestActorApp3* pTestActorApp3 = new CTestActorApp3();
	pTestActorApp3->run(argc, argv, "test_actor_config3.xml");

	return 0;
}