#include "test_service3.h"
#include "test_service3_message_handler.h"

DEFINE_OBJECT(CTestService3, 1);

CTestService3::CTestService3()
	: m_pTestService3MessageHandler(nullptr)
{

}

CTestService3::~CTestService3()
{

}

bool CTestService3::onInit()
{
	PrintInfo("CTestService3::onInit");

	this->m_pTestService3MessageHandler = new CTestService3MessageHandler(this);
	this->m_pTestService3MessageHandler->init();

	return true;
}

void CTestService3::onFrame()
{
	//PrintInfo("CTestService2::onFrame");
}

void CTestService3::onQuit()
{
	PrintInfo("CTestService2::onQuit");
}

