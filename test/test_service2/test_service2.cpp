#include "test_service2.h"
#include "test_service2_message_handler.h"

CTestService2::CTestService2(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
	, m_pTestService2MessageHandler(nullptr)
	, m_pNormalProtobufSerializer(nullptr)
{

}

CTestService2::~CTestService2()
{

}

bool CTestService2::onInit()
{
	PrintInfo("CTestService2::onInit");

	this->m_pNormalProtobufSerializer = new CNormalProtobufSerializer();

	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer);

	this->setServiceMessageSerializer(0, eMST_Protobuf);

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

void CTestService2::release()
{
	delete this;
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CTestService2(sServiceBaseInfo, szConfigFileName);
}