#include "test_service1.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"

#include "libCoreCommon/when_all.h"

DEFINE_OBJECT(CTestService1, 1);

CTestService1::CTestService1()
{
	this->m_ticker1.setCallback(std::bind(&CTestService1::onTicker1, this, std::placeholders::_1));
	this->m_ticker2.setCallback(std::bind(&CTestService1::onTicker2, this, std::placeholders::_1));
	this->m_ticker3.setCallback(std::bind(&CTestService1::onTicker3, this, std::placeholders::_1));
	this->m_mapConnectFlag[2] = false;
	this->m_mapConnectFlag[3] = false;
}

CTestService1::~CTestService1()
{

}

bool CTestService1::onInit()
{
	PrintInfo("CTestService1::onInit");

	this->setServiceConnectCallback(std::bind(&CTestService1::onServiceConnect, this, std::placeholders::_1));
	this->setServiceDisconnectCallback(std::bind(&CTestService1::onServiceDisconnect, this, std::placeholders::_1));

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

void CTestService1::onTicker1(uint64_t nContext)
{
	PrintInfo("CTestService1::onTicker1");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg1> sFuture;
	this->getServiceInvoker()->async_call(eMTT_Service, 2, &msg1, sFuture);
	sFuture.then_r([this](const response_msg1* pMsg, uint32_t nErrorCode)
	{
		CFuture<response_msg2> sFuture;
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestService1::onTicker1 response1 time out");
			return sFuture;
		}

		PrintInfo("CTestService1::onTicker1 response1 id: %d name: %s", pMsg->id(), pMsg->name().c_str());

		request_msg2 msg2;
		msg2.set_id(pMsg->id());
		msg2.set_name(pMsg->name());

		this->getServiceInvoker()->async_call(eMTT_Service, 3, &msg2, sFuture);

		return sFuture;

	}).then([this](const response_msg2* pMsg, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestService1::onTicker1 response2 time out");
			return;
		}

		PrintInfo("CTestService1::onTicker1 response2 id: %d name: %s", pMsg->id(), pMsg->name().c_str());
	});
}

void CTestService1::onTicker2(uint64_t nContext)
{
	PrintInfo("CTestService1::onTicker2");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg1> sFuture1;
	this->getServiceInvoker()->async_call(eMTT_Service, 2, &msg1, sFuture1);

	request_msg2 msg2;
	msg2.set_id(200);
	msg2.set_name("bbb");

	CFuture<response_msg2> sFuture2;
	this->getServiceInvoker()->async_call(eMTT_Service, 3, &msg2, sFuture2);

	auto sFuture = whenAll(sFuture1, sFuture2);

	sFuture.then([this](const std::tuple<CFuture<response_msg1>, CFuture<response_msg2>>* all, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestService1::onTicker2 response time out");
			return;
		}

		std::tuple<CFuture<response_msg1>, CFuture<response_msg2>>& sAll = const_cast<std::tuple<CFuture<response_msg1>, CFuture<response_msg2>>&>(*all);
		auto pMsg1 = std::get<0>(sAll).getValue();
		auto pMsg2 = std::get<1>(sAll).getValue();
		if (nullptr == pMsg1 || nullptr == pMsg2)
			return;

		PrintInfo("CTestService1::onTicker2 response id1: %d id2: %d name1: %s name2: %s", pMsg1->id(), pMsg2->id(), pMsg1->name().c_str(), pMsg2->name().c_str());
	});
}

void CTestService1::onTicker3(uint64_t nContext)
{
	PrintInfo("CTestService1::onTicker3");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	this->getServiceInvoker()->async_call<response_msg1>(eMTT_Service, 2, &msg1, [this](const response_msg1* pMsg, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestService1::onTicker1 response1 time out");
			return;
		}

		PrintInfo("CTestService1::onTicker1 response1 id: %d name: %s", pMsg->id(), pMsg->name().c_str());

		request_msg2 msg2;
		msg2.set_id(pMsg->id());
		msg2.set_name(pMsg->name());

		this->getServiceInvoker()->async_call<response_msg2>(eMTT_Service, 3, &msg2, [this](const response_msg2* pMsg, uint32_t nErrorCode)
		{
			if (nErrorCode != eRRT_OK)
			{
				PrintInfo("CTestService1::onTicker1 response2 time out");
				return;
			}

			PrintInfo("CTestService1::onTicker1 response2 id: %d name: %s", pMsg->id(), pMsg->name().c_str());
		});

	});
}

void CTestService1::onServiceConnect(uint32_t nServiceID)
{
	PrintInfo("ServiceConnect service_id: %d", nServiceID);

	this->m_mapConnectFlag[nServiceID] = true;

	for (auto iter = this->m_mapConnectFlag.begin(); iter != this->m_mapConnectFlag.end(); ++iter)
	{
		if (!iter->second)
			return;
	}

	this->registerTicker(&this->m_ticker1, 1000, 5000, 0);
	this->registerTicker(&this->m_ticker2, 1000, 5000, 0);
	this->registerTicker(&this->m_ticker3, 1000, 5000, 0);
}

void CTestService1::onServiceDisconnect(uint32_t nServiceID)
{
	PrintInfo("ServiceDisconnect service_id: %d", nServiceID);

	this->m_mapConnectFlag[nServiceID] = false;

	this->unregisterTicker(&this->m_ticker1);
	this->unregisterTicker(&this->m_ticker2);
	this->unregisterTicker(&this->m_ticker3);
}