#include "test_service2.h"
#include "test_service2_message_handler.h"
#include "libBaseCommon/time_util.h"

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

	this->setServiceMessageSerializer("", eMST_Protobuf);

	this->m_pTestService2MessageHandler = new CTestService2MessageHandler(this);
	
	PrintInfo(" #########  for 性能： 对撞 :  中间不输出 屏幕。只有前后输出。  ######### ");
	
	{
		this->m_ticker.setCallback(std::bind(&CTestService2::TestTickCallBack0, this, std::placeholders::_1));
		this->registerTicker(&(this->m_ticker), 3000, 10, 99999);

		this->m_nLastTime = base::time_util::getGmtTime();


		//int32_t ntmp = 0xff00;
		this->m_vecTickData.resize(0xff0);
		for (size_t i = 0; i < this->m_vecTickData.size(); ++i)
		{
			this->m_vecTickData[i].nFlag = 0;
			this->m_vecTickData[i].ticker.setCallback(std::bind(&CTestService2::TestTickCallBack3, this, std::placeholders::_1));
			this->registerTicker(&(this->m_vecTickData[i].ticker), 3000, 10, i);
		}
	}

	return true;
}

void CTestService2::onFrame()
{
	//PrintInfo("CTestService2::onFrame");
}

void CTestService2::onQuit()
{
	PrintInfo("CTestService2::onQuit");
	this->doQuit();
}

void CTestService2::TestTickCallBack3(uint64_t ucontext)
{
	this->m_vecTickData[ucontext].nFlag++;

	if ((this->m_vecTickData[ucontext].nFlag % 3) == 0)
	{
 		this->unregisterTicker(&(this->m_vecTickData[ucontext].ticker));
 		//m_pTickArry_2[ucontext].ticker.setCallback(std::bind(&CBussSrv::TestTickCallBack3, this, std::placeholders::_1));
 		this->registerTicker(&(this->m_vecTickData[ucontext].ticker), 10, 10, ucontext);
	}
}

void CTestService2::TestTickCallBack0(uint64_t ucontext)
{
	int64_t nCurTime = base::time_util::getGmtTime();
	//PrintInfo(" 时钟5 的再次触发间隔时间 {} 毫秒，预期 10ms ", nCurTime - this->m_nLastTime);
	this->m_nLastTime = nCurTime;
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