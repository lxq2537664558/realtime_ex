#include "test_actor1.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"

#include "libCoreCommon/when_all.h"
#include "libCoreCommon/base_app.h"

DEFINE_OBJECT(CTestActor1, 1);

CTestActor1::CTestActor1()
	: m_pServiceBase(nullptr)
{
	this->m_ticker1.setCallback(std::bind(&CTestActor1::onTicker1, this, std::placeholders::_1));
	this->m_ticker2.setCallback(std::bind(&CTestActor1::onTicker2, this, std::placeholders::_1));
	this->m_ticker3.setCallback(std::bind(&CTestActor1::onTicker3, this, std::placeholders::_1));
}

CTestActor1::~CTestActor1()
{

}

void CTestActor1::onInit(const std::string& szContext)
{
	PrintInfo("CTestActor1::onInit");

	uint32_t nServiceID = 0;
	base::crt::atoui(szContext.c_str(), nServiceID);
	this->m_pServiceBase = CBaseApp::Inst()->getServiceBase(nServiceID);
	DebugAst(this->m_pServiceBase != nullptr);

	this->registerTicker(&this->m_ticker1, 1000, 5000, 0);
	this->registerTicker(&this->m_ticker2, 1000, 5000, 0);
	this->registerTicker(&this->m_ticker3, 1000, 5000, 0);
}

void CTestActor1::onDestroy()
{
	PrintInfo("CTestActor1::onDestroy");
}

void CTestActor1::onTicker1(uint64_t nContext)
{
	PrintInfo("CTestActor1::onTicker1");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg1> sFuture;
	this->async_call(eMTT_Service, 2, &msg1, sFuture);
	sFuture.then_r([this](const response_msg1* pMsg, uint32_t nErrorCode)
	{
		CFuture<response_msg2> sFuture;
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestActor1::onTicker1 response1 time out");
			return sFuture;
		}

		PrintInfo("CTestActor1::onTicker1 response1 id: %d name: %s", pMsg->id(), pMsg->name().c_str());

		request_msg2 msg2;
		msg2.set_id(pMsg->id());
		msg2.set_name(pMsg->name());

		this->async_call(eMTT_Service, 3, &msg2, sFuture);

		return sFuture;

	}).then([this](const response_msg2* pMsg, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestActor1::onTicker1 response2 time out");
			return;
		}

		PrintInfo("CTestActor1::onTicker1 response2 id: %d name: %s", pMsg->id(), pMsg->name().c_str());
	});
}

void CTestActor1::onTicker2(uint64_t nContext)
{
	PrintInfo("CTestActor1::onTicker2");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg1> sFuture1;
	this->async_call(eMTT_Service, 2, &msg1, sFuture1);

	request_msg2 msg2;
	msg2.set_id(200);
	msg2.set_name("bbb");

	CFuture<response_msg2> sFuture2;
	this->async_call(eMTT_Service, 3, &msg2, sFuture2);

	auto sFuture = whenAll(sFuture1, sFuture2);

	sFuture.then([this](const std::tuple<CFuture<response_msg1>, CFuture<response_msg2>>* all, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestActor1::onTicker2 response time out");
			return;
		}

		std::tuple<CFuture<response_msg1>, CFuture<response_msg2>>& sAll = const_cast<std::tuple<CFuture<response_msg1>, CFuture<response_msg2>>&>(*all);
		auto pMsg1 = std::get<0>(sAll).getValue();
		auto pMsg2 = std::get<1>(sAll).getValue();
		if (nullptr == pMsg1 || nullptr == pMsg2)
			return;

		PrintInfo("CTestActor1::onTicker2 response id1: %d id2: %d name1: %s name2: %s", pMsg1->id(), pMsg2->id(), pMsg1->name().c_str(), pMsg2->name().c_str());
	});
}

void CTestActor1::onTicker3(uint64_t nContext)
{
	PrintInfo("CTestActor1::onTicker3");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	std::shared_ptr<response_msg1> pResponse1;
	this->sync_call(2, &msg1, pResponse1);
	if (pResponse1 == nullptr)
	{
		PrintInfo("pResponse1 == nullptr");
		return;
	}

	request_msg2 msg2;
	msg2.set_id(200);
	msg2.set_name("bbb");

	std::shared_ptr<response_msg2> pResponse2;
	this->sync_call(3, &msg2, pResponse2);
	if (pResponse2 == nullptr)
	{
		PrintInfo("pResponse2 == nullptr");
		return;
	}
	
	PrintInfo("CTestActor1::onTicker3 response id1: %d id2: %d name1: %s name2: %s", pResponse1->id(), pResponse2->id(), pResponse1->name().c_str(), pResponse2->name().c_str());
}