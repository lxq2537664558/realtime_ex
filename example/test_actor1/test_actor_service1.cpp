#include "test_actor_service1.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"

#include "libCoreCommon/when_all.h"
#include "test_actor0_message_handler.h"

DEFINE_OBJECT(CTestActorService1, 1);

CTestActorService1::CTestActorService1()
	: m_pTestActor1(nullptr)
	, m_pTestActor0(nullptr)
	, m_pTestActor0MessageHandler(nullptr)
	, m_pMyActorIDConverter(nullptr)
	, m_pDefaultProtobufFactory(nullptr)
{
}

CTestActorService1::~CTestActorService1()
{

}

bool CTestActorService1::onInit()
{
	PrintInfo("CTestActorService1::onInit");

	this->m_pTestActor0MessageHandler = new CTestActor0MessageHandler(this);
	
	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	this->setProtobufFactory(this->m_pDefaultProtobufFactory);

	this->m_pMyActorIDConverter = new CMyActorIDConverter();
	this->setActorIDConverter(this->m_pMyActorIDConverter);

	CTestActor1::registerClassInfo();
	CTestActor0::registerClassInfo();

	this->setServiceConnectCallback(std::bind(&CTestActorService1::onServiceConnect, this, std::placeholders::_1));
	this->setServiceDisconnectCallback(std::bind(&CTestActorService1::onServiceDisconnect, this, std::placeholders::_1));

	return true;
}

void CTestActorService1::onFrame()
{
	//PrintInfo("CTestService1::onFrame");
}

void CTestActorService1::onQuit()
{
	PrintInfo("CTestService1::onQuit");
}

void CTestActorService1::onServiceConnect(uint32_t nServiceID)
{
	PrintInfo("ServiceConnect service_id: %d", nServiceID);

	char szBuf[256] = {};
	base::crt::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	CActorBase* pActorBase = this->createActor("CTestActor1", 1, szBuf);
	DebugAst(pActorBase != nullptr);

	this->m_pTestActor1 = dynamic_cast<CTestActor1*>(pActorBase);

	pActorBase = this->createActor("CTestActor0", 3, szBuf);
	DebugAst(pActorBase != nullptr);

	this->m_pTestActor0 = dynamic_cast<CTestActor0*>(pActorBase);
}

void CTestActorService1::onServiceDisconnect(uint32_t nServiceID)
{
	PrintInfo("ServiceDisconnect service_id: %d", nServiceID);
}