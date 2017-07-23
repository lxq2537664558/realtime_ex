#include "test_actor_service2.h"
#include "test_actor_service2_message_handler.h"
#include "test_actor2_message_handler.h"

DEFINE_OBJECT(CTestActorService2, 1);

CTestActorService2::CTestActorService2()
	: m_pTestActorService2MessageHandler(nullptr)
	, m_pTestActor2MessageHandler(nullptr)
	, m_pTestActor2(nullptr)
{
}

CTestActorService2::~CTestActorService2()
{

}

bool CTestActorService2::onInit()
{
	PrintInfo("CTestActorService2::onInit");

	CTestActor2::registerClassInfo();

	this->m_pTestActorService2MessageHandler = new CTestActorService2MessageHandler(this);
	this->m_pTestActorService2MessageHandler->init();

	this->m_pTestActor2MessageHandler = new CTestActor2MessageHandler(this);
	this->m_pTestActor2MessageHandler->init();

	char szBuf[256] = {};
	base::crt::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	CActorBase* pActorBase = this->createActor("CTestActor2", 2, szBuf);
	DebugAstEx(pActorBase != nullptr, false);

	this->m_pTestActor2 = dynamic_cast<CTestActor2*>(pActorBase);

	return true;
}

void CTestActorService2::onFrame()
{
	//PrintInfo("CTestActorService2::onFrame");
}

void CTestActorService2::onQuit()
{
	PrintInfo("CTestActorService2::onQuit");
}