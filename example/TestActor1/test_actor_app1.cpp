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

class CTestActor1 : public core::CBaseActor
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
		this->registerCallback(eServiceRequestActor1, std::bind(&CTestActor1::onRequest, this, std::placeholders::_1, std::placeholders::_2));
		
		return true;
	}

	void onRequest(uint64_t nFrom, core::CMessage pMessage)
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

private:
	uint32_t	m_nID;
};

class CTestActor2 : public core::CBaseActor
{
public:
	CTestActor2()
	{
		
	}

	virtual ~CTestActor2()
	{

	}

	virtual bool onInit(void* pContext)
	{
		this->registerCallback(eServiceRequestActor2, std::bind(&CTestActor2::onRequest, this, std::placeholders::_1, std::placeholders::_2));

		return true;
	}

	void onRequest(uint64_t nFrom, core::CMessage pMessage)
	{
		CServiceRequestActor3 request3;
		request3.nID = reinterpret_cast<const CServiceRequestActor2*>(pMessage.get())->nID;

		core::CResponseFuture sResponseFuture;
		this->invoke_r(this->m_nDstActorID, &request3, sResponseFuture);
		core::SActorSessionInfo sActorSessionInfo = this->getActorSessionInfo();
		sResponseFuture.then_r([this](core::CMessage pMessage, uint32_t nErrorCode)
		{
			CServiceRequestActor4 request4;
			request4.nID = reinterpret_cast<const CServiceRequestActor3*>(pMessage.get())->nID;
			core::CResponseFuture sResponseFuture;
			this->invoke_r(this->m_nDstActorID, &request4, sResponseFuture);

			return sResponseFuture;
		}).then([this, sActorSessionInfo](core::CMessage pMessage, uint32_t nErrorCode)
		{
			CServiceResponseActor2 netMsg;
			netMsg.nID = reinterpret_cast<const CServiceRequestActor4*>(pMessage.get())->nID;
			this->response(sActorSessionInfo, &netMsg);
		});
	}

	uint64_t m_nDstActorID;
};

class CTestActor3 : public core::CBaseActor
{
public:
	CTestActor3()
	{

	}

	virtual ~CTestActor3()
	{

	}

	virtual bool onInit(void* pContext)
	{
		this->registerCallback(eServiceRequestActor3, std::bind(&CTestActor3::onRequest3, this, std::placeholders::_1, std::placeholders::_2));
		this->registerCallback(eServiceRequestActor4, std::bind(&CTestActor3::onRequest4, this, std::placeholders::_1, std::placeholders::_2));

		return true;
	}

	void onRequest3(uint64_t nFrom, core::CMessage pMessage)
	{
		CServiceResponseActor3 netMsg;
		netMsg.nID = reinterpret_cast<const CServiceRequestActor3*>(pMessage.get())->nID;

		this->response(&netMsg);
	}

	void onRequest4(uint64_t nFrom, core::CMessage pMessage)
	{
		CServiceResponseActor4 netMsg;
		netMsg.nID = reinterpret_cast<const CServiceRequestActor4*>(pMessage.get())->nID;

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