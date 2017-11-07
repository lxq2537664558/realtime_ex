#include "test_service3.h"
#include "test_service3_message_handler.h"

CTestService3::CTestService3(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
	, m_pTestService3MessageHandler(nullptr)
	, m_pNormalProtobufSerializer(nullptr)
{

}

CTestService3::~CTestService3()
{

}

void CTestService3::release()
{
	delete this;
}

bool CTestService3::onInit()
{
	PrintInfo("CTestService3::onInit");

	this->m_pNormalProtobufSerializer = new CNormalProtobufSerializer();

	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer);

	this->setServiceMessageSerializer("", eMST_Protobuf);
	
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
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CTestService3(sServiceBaseInfo, szConfigFileName);
}