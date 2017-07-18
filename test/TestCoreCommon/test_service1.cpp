#include "test_service1.h"
#include "request_msg1.pb.h"
#include "response_msg1.pb.h"

DEFINE_OBJECT(CTestService1, 1);

CTestService1::CTestService1()
{
	this->m_ticker.setCallback(std::bind(&CTestService1::onTicker, this, std::placeholders::_1));
}

CTestService1::~CTestService1()
{

}

bool CTestService1::onInit()
{
	PrintInfo("CTestService1::onInit");

	this->registerTicker(&this->m_ticker, 1000, 5000, 0);

	return true;
}

void CTestService1::onFrame()
{
	//PrintInfo("CTestService1::onFrame");
}

void CTestService1::onQuit()
{
	PrintInfo("CTestService1::onQuit");
}

void CTestService1::onTicker(uint64_t nContext)
{
	PrintInfo("CTestService1::onTicker");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg1> sFuture;
	this->getServiceInvoker()->async_call(eMTT_Service, 2, &msg1, sFuture);
	sFuture.then([](const response_msg1* pMsg, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("response time out");
			return;
		}

		PrintInfo("response id: %d name: %s", pMsg->id(), pMsg->name().c_str());
	});
}