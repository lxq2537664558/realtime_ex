// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_actor_app1.h"

#include "libCoreCommon/base_app.h"
#include "libCoreServiceKit/cluster_invoker.h"
#include "libCoreServiceKit/core_service_app.h"

#include "../common/test_message_define1.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/memory_hook.h"
#include "libCoreServiceKit/base_actor.h"
#include "libCoreServiceKit/base_actor_factory.h"

#include <memory>
#include "libCoreServiceKit/actor_message_registry.h"
#include "libCoreServiceKit/message_ptr.h"

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

class CTestActor1 
	: public core::CBaseActor
	, public core::CActorMessageRegistry<CTestActor1>
{
public:
	CTestActor1()
	{
		m_nID = 0;
	}

	virtual ~CTestActor1()
	{

	}

	DEFEND_ACTOR_MESSAGE_FUNCTION(CTestActor1)

	virtual bool onInit(void* pContext)
	{
		REGISTER_ACTOR_MESSAGE_HANDLER(CTestActor1, eServiceRequestActor1, &CTestActor1::onRequest, false);
		return true;
	}

	void onRequest(CBaseActor* pBaseActor, uint64_t nFrom, core::CMessagePtr<CServiceRequestActor1> pMessage)
	{
		uint64_t nActorID = pMessage->nActorID;
		//PrintInfo("Time2: "UINT64FMT, base::getProcessPassTime());
		while (true)
		{
			CServiceRequestActor2 netMsg;
			netMsg.nID = this->m_nID++;
			core::CMessagePtr<CServiceResponseActor2> pResultData;
			int64_t nBeginTime = base::getProcessPassTime();
			this->invoke(nActorID, &netMsg, pResultData);
			int64_t nEndTime = base::getProcessPassTime();

			PrintInfo("Actor1 ID: %d cost: "UINT64FMT, pResultData->nID, nEndTime - nBeginTime);
		}
	}

private:
	uint32_t	m_nID;
};

class CTestActor2
	: public core::CBaseActor
	, public core::CActorMessageRegistry<CTestActor2>
{
public:
	CTestActor2()
	{
		
	}

	virtual ~CTestActor2()
	{

	}

	DEFEND_ACTOR_MESSAGE_FUNCTION(CTestActor2)

	virtual bool onInit(void* pContext)
	{
		REGISTER_ACTOR_MESSAGE_HANDLER(CTestActor2, eServiceRequestActor2, &CTestActor2::onRequest, true);

		return true;
	}

	void onRequest(CBaseActor* pBaseActor, uint64_t nFrom, core::CMessagePtr<CServiceRequestActor2> pMessage)
	{
		CServiceRequestActor3 request3;
		request3.nID = pMessage->nID;
		//PrintInfo("Time3: "UINT64FMT, base::getProcessPassTime());
		core::CFuture<core::CMessagePtr<CServiceRequestActor3>> sResponseFuture;
		this->invoke_r(this->m_nDstActorID, &request3, sResponseFuture);
		core::SActorSessionInfo sActorSessionInfo = this->getActorSessionInfo();
		sResponseFuture.then_r([this](core::CMessagePtr<CServiceRequestActor3> pMessage, uint32_t nErrorCode)
		{
			//PrintInfo("Time6: "UINT64FMT, base::getProcessPassTime());
			CServiceRequestActor4 request4;
			request4.nID = pMessage->nID;
			core::CFuture<core::CMessagePtr<CServiceRequestActor4>> sResponseFuture;
			this->invoke_r(this->m_nDstActorID, &request4, sResponseFuture);

			return sResponseFuture;
		}).then([this, sActorSessionInfo](core::CMessagePtr<CServiceRequestActor4> pMessage, uint32_t nErrorCode)
		{
			//PrintInfo("Time7: "UINT64FMT, base::getProcessPassTime());
			CServiceResponseActor2 netMsg;
			netMsg.nID = pMessage->nID;
			this->response(sActorSessionInfo, &netMsg);
		});
	}

	uint64_t m_nDstActorID;
};

class CTestActor3
	: public core::CBaseActor
	, public core::CActorMessageRegistry<CTestActor3>
{
public:
	CTestActor3()
	{

	}

	virtual ~CTestActor3()
	{

	}

	DEFEND_ACTOR_MESSAGE_FUNCTION(CTestActor3)

	virtual bool onInit(void* pContext)
	{
		REGISTER_ACTOR_MESSAGE_HANDLER(CTestActor3, eServiceRequestActor3, &CTestActor3::onRequest3, true);
		REGISTER_ACTOR_MESSAGE_HANDLER(CTestActor3, eServiceRequestActor4, &CTestActor3::onRequest4, true);
		return true;
	}

	void onRequest3(CBaseActor* pBaseActor, uint64_t nFrom, core::CMessagePtr<CServiceRequestActor3> pMessage)
	{
		CServiceResponseActor3 netMsg;
		netMsg.nID = pMessage->nID;
		//PrintInfo("Time4: "UINT64FMT, base::getProcessPassTime());
		this->response(&netMsg);
	}

	void onRequest4(CBaseActor* pBaseActor, uint64_t nFrom, core::CMessagePtr<CServiceRequestActor4> pMessage)
	{
		CServiceResponseActor4 netMsg;
		netMsg.nID = pMessage->nID;
		//PrintInfo("Time5: "UINT64FMT, base::getProcessPassTime());
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

class CTestActorFactory2 :
	public core::CBaseActorFactory
{
public:
	core::CBaseActor* createBaseActor()
	{
		return new CTestActor2();
	}
};

class CTestActorFactory3 :
	public core::CBaseActorFactory
{
public:
	core::CBaseActor* createBaseActor()
	{
		return new CTestActor3();
	}
};

bool CTestActorApp1::onInit()
{
	CCoreServiceApp::onInit();

	CTestActorFactory1* pTestActorFactory1 = new CTestActorFactory1();
	CTestActorFactory2* pTestActorFactory2 = new CTestActorFactory2();
	CTestActorFactory3* pTestActorFactory3 = new CTestActorFactory3();

	core::CBaseActor* pActor1 = core::CBaseActor::createActor("", pTestActorFactory1);
	core::CBaseActor* pActor2 = core::CBaseActor::createActor("", pTestActorFactory2);
	core::CBaseActor* pActor3 = core::CBaseActor::createActor("", pTestActorFactory3);

	dynamic_cast<CTestActor2*>(pActor2)->m_nDstActorID = pActor3->getID();

	for (size_t i = 0; i < 1; ++i)
	{
		CServiceRequestActor1 netMsg;
		//PrintInfo("Time1: "UINT64FMT, base::getProcessPassTime());
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