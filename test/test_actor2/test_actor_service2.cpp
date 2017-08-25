#include "test_actor_service2.h"
#include "test_actor_service2_message_handler.h"
#include "test_actor2_message_handler.h"

CTestActorService2::CTestActorService2()
	: m_pTestActorService2MessageHandler(nullptr)
	, m_pTestActor2MessageHandler(nullptr)
	, m_pTestActor2(nullptr)
	, m_pMyActorFactory(nullptr)
	, m_pNormalProtobufFactory(nullptr)
{
}

CTestActorService2::~CTestActorService2()
{

}

CProtobufFactory* CTestActorService2::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

CActorFactory* CTestActorService2::getActorFactory(const std::string& szType) const
{
	return this->m_pMyActorFactory;
}

void CTestActorService2::release()
{
	delete this;
}

bool CTestActorService2::onInit()
{
	PrintInfo("CTestActorService2::onInit");

	this->m_pTestActorService2MessageHandler = new CTestActorService2MessageHandler(this);
	
	this->m_pTestActor2MessageHandler = new CTestActor2MessageHandler(this);
	
	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	this->m_pMyActorFactory = new CMyActorFactory();

	char szBuf[256] = {};
	base::function_util::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

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

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif 
CServiceBase* createServiceBase()
{
	return new CTestActorService2();
}