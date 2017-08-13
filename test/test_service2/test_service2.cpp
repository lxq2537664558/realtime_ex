#include "test_service2.h"
#include "test_service2_message_handler.h"

CTestService2::CTestService2()
	: m_pTestService2MessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
{

}

CTestService2::~CTestService2()
{

}

bool CTestService2::onInit()
{
	PrintInfo("CTestService2::onInit");

	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	
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

CProtobufFactory* CTestService2::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

void CTestService2::release()
{
	delete this;
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CTestService2();
}