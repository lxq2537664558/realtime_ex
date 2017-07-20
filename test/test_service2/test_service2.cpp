#include "test_service2.h"
#include "test_service2_message_handler.h"

DEFINE_OBJECT(CTestService2, 1);

CTestService2::CTestService2()
	: m_pTestService2MessageHandler(nullptr)
{

}

CTestService2::~CTestService2()
{

}

bool CTestService2::onInit()
{
	PrintInfo("CTestService2::onInit");

	this->m_pTestService2MessageHandler = new CTestService2MessageHandler(this);
	this->m_pTestService2MessageHandler->init();

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

