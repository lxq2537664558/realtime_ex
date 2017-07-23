#include "test_actor1.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"
#include "../proto_src/request_msg0.pb.h"
#include "../proto_src/request_msg3.pb.h"

#include "libCoreCommon/when_all.h"
#include "libCoreCommon/base_app.h"
#include "../proto_src/response_msg0.pb.h"
#include "../proto_src/response_msg3.pb.h"

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

	request_msg0 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg0> sFuture;
	this->async_call(eMTT_Actor, 3, &msg1, sFuture);
	sFuture.then_r([this](const response_msg0* pMsg, uint32_t nErrorCode)
	{
		CFuture<response_msg3> sFuture;
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestActor1::onTicker1 response0 time out");
			return sFuture;
		}

		PrintInfo("CTestActor1::onTicker1 response0 id: %d name: %s", pMsg->id(), pMsg->name().c_str());

		request_msg3 msg2;
		msg2.set_id(pMsg->id());
		msg2.set_name(pMsg->name());

		this->async_call(eMTT_Actor, 2, &msg2, sFuture);

		return sFuture;

	}).then([this](const response_msg3* pMsg, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestActor1::onTicker1 response3 time out");
			return;
		}

		PrintInfo("CTestActor1::onTicker1 response3 id: %d name: %s", pMsg->id(), pMsg->name().c_str());
	});
}

void CTestActor1::onTicker2(uint64_t nContext)
{
	PrintInfo("CTestActor1::onTicker2");

	request_msg0 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg0> sFuture1;
	this->async_call(eMTT_Actor, 3, &msg1, sFuture1);

	request_msg3 msg2;
	msg2.set_id(200);
	msg2.set_name("bbb");

	CFuture<response_msg3> sFuture2;
	this->async_call(eMTT_Actor, 2, &msg2, sFuture2);

	auto sFuture = whenAll(sFuture1, sFuture2);

	sFuture.then([this](const std::tuple<CFuture<response_msg0>, CFuture<response_msg3>>* all, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestActor1::onTicker2 response time out");
			return;
		}

		std::tuple<CFuture<response_msg0>, CFuture<response_msg3>>& sAll = const_cast<std::tuple<CFuture<response_msg0>, CFuture<response_msg3>>&>(*all);
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

	request_msg0 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	this->async_call<response_msg0>(eMTT_Actor, 3, &msg1, [&](const response_msg0* pMsg, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestActor1::onTicker3 response3 time out");
			return;
		}

		PrintInfo("CTestActor1::onTicker3 response0 id: %d name: %s", pMsg->id(), pMsg->name().c_str());
	});

	request_msg2 msg2;
	msg2.set_id(200);
	msg2.set_name("bbb");

	std::shared_ptr<response_msg2> pResponse2;
	this->sync_call(5, &msg2, pResponse2);
	if (pResponse2 == nullptr)
	{
		PrintInfo("pResponse2 == nullptr");
		return;
	}

	PrintInfo("CTestActor1::onTicker3 response2 id: %d name: %s", pResponse2->id(), pResponse2->name().c_str());
}