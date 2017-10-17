#include "test_service1.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"

#include "libCoreCommon/when_all.h"
#include "libCoreCommon/service_invoker.h"
#include "libBaseCommon/rand_gen.h"
#include "libBaseCommon/memory_hook.h"
#include "libBaseCommon/time_util.h"
#include <thread>

// #include <sys/syscall.h>
// #include <pthread.h>

CTestService1::CTestService1(const core::SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
	: CServiceBase(sServiceBaseInfo, szConfigFileName)
	, m_pNormalProtobufSerializer(nullptr)
{
	this->m_ticker1.setCallback(std::bind(&CTestService1::onTicker1, this, std::placeholders::_1));
	this->m_ticker2.setCallback(std::bind(&CTestService1::onTicker2, this, std::placeholders::_1));
	this->m_ticker3.setCallback(std::bind(&CTestService1::onTicker3, this, std::placeholders::_1));
	this->m_mapConnectFlag[2] = false;
	this->m_mapConnectFlag[3] = false;

	this->m_ticker0.resize(100);
	for (size_t i = 0; i < this->m_ticker0.size(); ++i)
	{
		this->m_ticker0[i].setCallback(std::bind(&CTestService1::onTicker0, this, std::placeholders::_1));
	}
}

CTestService1::~CTestService1()
{

}

bool CTestService1::onInit()
{
	PrintInfo("CTestService1::onInit");

	this->m_pNormalProtobufSerializer = new CNormalProtobufSerializer();
	
	this->addServiceMessageSerializer(this->m_pNormalProtobufSerializer);

	this->setServiceMessageSerializer(0, eMST_Protobuf);

	this->setServiceConnectCallback(std::bind(&CTestService1::onServiceConnect, this, std::placeholders::_1, std::placeholders::_2));
	this->setServiceDisconnectCallback(std::bind(&CTestService1::onServiceDisconnect, this, std::placeholders::_1, std::placeholders::_2));

	for (size_t i = 0; i < this->m_ticker0.size(); ++i)
	{
		uint32_t nRand = base::CRandGen::getGlobalRand(500) + 500;
		this->registerTicker(&this->m_ticker0[i], nRand, nRand, 0);
	}

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

void CTestService1::release()
{
	delete this;
}

void CTestService1::onTicker0(uint64_t nContext)
{
	//while(true)
	{
		request_msg1 msg1;
		msg1.set_id(100);
		msg1.set_name("aaa");

		//PrintInfo("call_begin {}", syscall(SYS_gettid));

		std::shared_ptr<response_msg1> pResponseMsg1;
		if (this->getServiceInvoker()->sync_invoke<response_msg1>("test_service", eSST_Random, 0, &msg1, pResponseMsg1) != eRRT_OK)
		{
			PrintInfo("AAAAAAAAA");
		}
		else
		{
			PrintInfo("BBBBBBBBB");
		}

		PrintInfo("co_stack_size: {} co_count: {}", core::coroutine::getTotalStackSize(), core::coroutine::getCoroutineCount());
		//PrintInfo("call_end {}", syscall(SYS_gettid));
		//PrintInfo("{}", GetCurrentThreadId());
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}


// 	{
// 		request_msg1 msg1;
// 		msg1.set_id(100);
// 		msg1.set_name("aaa");
// 
// 		this->getServiceInvoker()->async_invoke<response_msg1>("test_service", eSST_Random, 0, &msg1, [](const response_msg1*, uint32_t nErrorCode)
// 		{
// 			if (nErrorCode != eRRT_OK)
// 				PrintInfo("AAAAAAAAA");
// 			else
// 				PrintInfo("BBBBBBBBB");
// 
// 			PrintInfo("call_end {}", syscall(SYS_gettid));
// 		});
// 		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
// 	}
}

void CTestService1::onTicker1(uint64_t nContext)
{
	PrintInfo("CTestService1::onTicker1");
	base::endMemoryLeakChecker("aaa.txt");
	for (size_t i = 0; i < 10000; ++i)
	{
		request_msg1 msg1;
		msg1.set_id(100);
		msg1.set_name("aaa");

		CFuture<response_msg1> sFuture;
		this->getServiceInvoker()->async_invoke(2, &msg1, sFuture);
		sFuture.then_r([this](const response_msg1* pMsg, uint32_t nErrorCode)
		{
			CFuture<response_msg2> sFuture;
			if (nErrorCode != eRRT_OK)
			{
				PrintInfo("CTestService1::onTicker1 response1 error {}", nErrorCode);
				return sFuture;
			}

			//PrintInfo("CTestService1::onTicker1 response1 id: {} name: {}", pMsg->id(), pMsg->name().c_str());

			request_msg2 msg2;
			msg2.set_id(pMsg->id());
			msg2.set_name(pMsg->name());

			this->getServiceInvoker()->async_invoke(3, &msg2, sFuture);

			return sFuture;

		}).then([this](const response_msg2* pMsg, uint32_t nErrorCode)
		{
			if (nErrorCode != eRRT_OK)
			{
				PrintInfo("CTestService1::onTicker1 response2 time out");
				return;
			}

			//PrintInfo("CTestService1::onTicker1 response2 id: {} name: {}", pMsg->id(), pMsg->name().c_str());
		});
	}
}

void CTestService1::onTicker2(uint64_t nContext)
{
	PrintInfo("CTestService1::onTicker2");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	CFuture<response_msg1> sFuture1;
	this->getServiceInvoker()->async_invoke(2, &msg1, sFuture1);

	request_msg2 msg2;
	msg2.set_id(200);
	msg2.set_name("bbb");

	CFuture<response_msg2> sFuture2;
	this->getServiceInvoker()->async_invoke(3, &msg2, sFuture2);

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

		PrintInfo("CTestService1::onTicker2 response id1: {} id2: {} name1: {} name2: {}", pMsg1->id(), pMsg2->id(), pMsg1->name().c_str(), pMsg2->name().c_str());
	});
}

void CTestService1::onTicker3(uint64_t nContext)
{
	PrintInfo("CTestService1::onTicker3");

	request_msg1 msg1;
	msg1.set_id(100);
	msg1.set_name("aaa");

	this->getServiceInvoker()->async_invoke<response_msg1>(2, &msg1, [this](const response_msg1* pMsg, uint32_t nErrorCode)
	{
		if (nErrorCode != eRRT_OK)
		{
			PrintInfo("CTestService1::onTicker1 response1 time out");
			return;
		}

		PrintInfo("CTestService1::onTicker1 response1 id: {} name: {}", pMsg->id(), pMsg->name().c_str());

		request_msg2 msg2;
		msg2.set_id(pMsg->id());
		msg2.set_name(pMsg->name());

		this->getServiceInvoker()->async_invoke<response_msg2>(3, &msg2, [this](const response_msg2* pMsg, uint32_t nErrorCode)
		{
			if (nErrorCode != eRRT_OK)
			{
				PrintInfo("CTestService1::onTicker1 response2 time out");
				return;
			}

			PrintInfo("CTestService1::onTicker1 response2 id: {} name: {}", pMsg->id(), pMsg->name().c_str());
		});

	});
}

void CTestService1::onServiceConnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "master")
		return;

	PrintInfo("ServiceConnect service_id: {}", nServiceID);

	this->m_mapConnectFlag[nServiceID] = true;

	for (auto iter = this->m_mapConnectFlag.begin(); iter != this->m_mapConnectFlag.end(); ++iter)
	{
		if (!iter->second)
			return;
	}

// 	for (size_t i = 0; i < this->m_ticker0.size(); ++i)
// 	{
// 		uint32_t nRand = base::CRandGen::getGlobalRand(500) + 500;
// 		this->registerTicker(&this->m_ticker0[i], nRand, 0, 0);
// 	}
// 	this->registerTicker(&this->m_ticker1, 10000, 0, 0);
// 	base::beginMemoryLeakChecker();
// 	this->registerTicker(&this->m_ticker2, 1000, 5000, 0);
// 	this->registerTicker(&this->m_ticker3, 1000, 5000, 0);
}

void CTestService1::onServiceDisconnect(const std::string& szType, uint32_t nServiceID)
{
	if (szType == "master")
		return;

	PrintInfo("ServiceDisconnect service_id: {}", nServiceID);

	this->m_mapConnectFlag[nServiceID] = false;

// 	this->unregisterTicker(&this->m_ticker1);
// 	this->unregisterTicker(&this->m_ticker2);
// 	this->unregisterTicker(&this->m_ticker3);
}

extern "C" 
#ifdef _WIN32
__declspec(dllexport)
#endif
CServiceBase* createServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName)
{
	return new CTestService1(sServiceBaseInfo, szConfigFileName);
}