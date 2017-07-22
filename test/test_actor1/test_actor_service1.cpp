#include "test_actor_service1.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "../proto_src/request_msg2.pb.h"

#include "libCoreCommon/when_all.h"

DEFINE_OBJECT(CTestActorService1, 1);

CTestActorService1::CTestActorService1()
	: m_pTestActor1(nullptr)
{
	this->m_mapConnectFlag[2] = false;
	this->m_mapConnectFlag[3] = false;
}

CTestActorService1::~CTestActorService1()
{

}

bool CTestActorService1::onInit()
{
	PrintInfo("CTestActorService1::onInit");
	CTestActor1::registerClassInfo();

	this->setServiceConnectCallback(std::bind(&CTestActorService1::onServiceConnect, this, std::placeholders::_1));
	this->setServiceDisconnectCallback(std::bind(&CTestActorService1::onServiceDisconnect, this, std::placeholders::_1));

	return true;
}

void CTestActorService1::onFrame()
{
	//PrintInfo("CTestService1::onFrame");
}

void CTestActorService1::onQuit()
{
	PrintInfo("CTestService1::onQuit");
}

void CTestActorService1::onServiceConnect(uint32_t nServiceID)
{
	PrintInfo("ServiceConnect service_id: %d", nServiceID);

	this->m_mapConnectFlag[nServiceID] = true;

	for (auto iter = this->m_mapConnectFlag.begin(); iter != this->m_mapConnectFlag.end(); ++iter)
	{
		if (!iter->second)
			return;
	}

	char szBuf[256] = {};
	base::crt::snprintf(szBuf, _countof(szBuf), "%d", this->getServiceID());

	CActorBase* pActorBase = this->createActor("CTestActor1", szBuf);
	DebugAst(pActorBase != nullptr);

	this->m_pTestActor1 = dynamic_cast<CTestActor1*>(pActorBase);
}

void CTestActorService1::onServiceDisconnect(uint32_t nServiceID)
{
	PrintInfo("ServiceDisconnect service_id: %d", nServiceID);

	this->m_mapConnectFlag[nServiceID] = false;

}