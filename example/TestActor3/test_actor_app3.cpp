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
#include "libCoreServiceKit/actor.h"

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
				CServiceRequestActor4 netMsg;
				netMsg.nID = this->m_nID++;
				core::CMessage pResultData;
				this->invoke(nActorID, &netMsg, pResultData);
				PrintInfo("Actor1 ID: %d", reinterpret_cast<const CServiceResponseActor4*>(pResultData.get())->nID);
			}
		}
	}

private:
	uint32_t	m_nID;
};

void onServiceConnect(uint16_t nServiceID)
{
	if (nServiceID != 7)
		return;

	core::CActor* pActor1 = new CTestActor1();

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

	this->setServiceConnectCallback(std::bind(&onServiceConnect, std::placeholders::_1));
	
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