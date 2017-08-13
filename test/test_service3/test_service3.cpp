#include "test_service3.h"
#include "test_service3_message_handler.h"

CTestService3::CTestService3()
	: m_pTestService3MessageHandler(nullptr)
	, m_pNormalProtobufFactory(nullptr)
{

}

CTestService3::~CTestService3()
{

}

CProtobufFactory* CTestService3::getServiceProtobufFactory() const
{
	return this->m_pNormalProtobufFactory;
}

void CTestService3::release()
{
	delete this;
}

bool CTestService3::onInit()
{
	PrintInfo("CTestService3::onInit");

	this->m_pNormalProtobufFactory = new CNormalProtobufFactory();
	
	this->m_pTestService3MessageHandler = new CTestService3MessageHandler(this);
	
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

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase()
{
	return new CTestService3();
}