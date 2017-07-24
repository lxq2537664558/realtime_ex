#include "test_service2.h"
#include "test_service2_message_handler.h"

DEFINE_OBJECT(CTestService2, 1);

CTestService2::CTestService2()
	: m_pTestService2MessageHandler(nullptr)
	, m_pDefaultProtobufFactory(nullptr)
{

}

CTestService2::~CTestService2()
{

}

bool CTestService2::onInit()
{
	PrintInfo("CTestService2::onInit");

	this->m_pDefaultProtobufFactory = new CDefaultProtobufFactory();
	this->setProtobufFactory(this->m_pDefaultProtobufFactory);

	this->m_pTestService2MessageHandler = new CTestService2MessageHandler(this);
	
	return true;
}

void CTestService2::onFrame()
{
	//PrintInfo("CTestService2::onFrame");
}

void CTestService2::onQuit()
{
	PrintInfo("CTestService2::onQuit");
}

