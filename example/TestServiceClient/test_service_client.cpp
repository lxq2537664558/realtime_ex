// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_client.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libBaseCommon/base_time.h"

#include "libCoreCommon/base_connection_factory.h"
#include "../common/test_message_define.h"
#include "libBaseCommon/rand_gen.h"
#include <functional>
#include <xfunctional>

#pragma pack(push,1)
// 消息头
struct message_header
{
	uint16_t	nMessageSize;	// 包括消息头的
	uint32_t	nMessageID;

	message_header(uint16_t nMessageID) : nMessageID(nMessageID) { }
	message_header() {}
};
#pragma pack(pop)

class CConnectToService :
	public core::CBaseConnection
{
public:
	CConnectToService()
	{
		nNextPacketID = 0;
		nClientID = 0;
	}

	virtual ~CConnectToService() 
	{
	}

	virtual bool		init(const std::string& szContext)
	{
		return true;
	}
	
	virtual uint32_t	getType() const { return _BASE_CONNECTION_TYPE_BEGIN; }
	virtual void		release(){ delete this; }
	
	virtual void onConnect()
	{
		PrintDebug("onConnect");
		this->setMessageParser(default_client_message_parser);
		this->nClientID = (++g_nClientID);
		this->requestMsg(1);
	}
	
	virtual void onDisconnect()
	{
		PrintDebug("onDisconnect");
	}

	virtual void onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		const SClientResponseMsg* pMsg = static_cast<const SClientResponseMsg*>(pData);
		if (pMsg->nClientID == 1)
			PrintInfo("onDispatch id: %d cost: %d", pMsg->nID, (uint32_t)(base::getGmtTime() - pMsg->nClientTime));
		this->requestMsg(pMsg->nID + 1);
	}

private:
	void requestMsg(uint32_t nID)
	{
		SClientRequestMsg msg;
		msg.nSync = 0;
		msg.nClientID = nClientID;
		msg.nID = nID;
		msg.nClientTime = base::getGmtTime();
		this->send(eMT_CLIENT, &msg, sizeof(msg));

		nSendTime = base::getGmtTime();
		++nNextPacketID;
	}

public:
	uint32_t	nClientID;
	uint64_t	nID;
	uint32_t	nNextPacketID;
	uint64_t	nSendTime;

	static uint32_t g_nClientID;
};

uint32_t CConnectToService::g_nClientID = 0;

class CServiceConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CServiceConnectionFactory() { }
	virtual ~CServiceConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(uint32_t nType, const std::string& szContext)
	{
		CConnectToService* pConnectToService = new CConnectToService();
		if (!pConnectToService->init(szContext))
		{
			SAFE_RELEASE(pConnectToService);
			return nullptr;
		}

		return pConnectToService;
	}
};

CTestServiceClientApp::CTestServiceClientApp()
{
}

CTestServiceClientApp::~CTestServiceClientApp()
{
}

CTestServiceClientApp* CTestServiceClientApp::Inst()
{
	return static_cast<CTestServiceClientApp*>(core::CBaseApp::Inst());
}

void fun(uint64_t nContext)
{
	exit(0);
}

bool CTestServiceClientApp::onInit()
{
	PrintDebug("aaaa");
	CServiceConnectionFactory* pServiceConnectionFactory = new CServiceConnectionFactory();
	this->getBaseConnectionMgr()->setBaseConnectionFactory(_BASE_CONNECTION_TYPE_BEGIN, pServiceConnectionFactory);
	for (size_t i = 0; i < 100; ++i)
	{
		this->getBaseConnectionMgr()->connect("127.0.0.1", 8000, _BASE_CONNECTION_TYPE_BEGIN, "", 0, 0);
	}

	core::CTicker* pTick = new core::CTicker();
	pTick->setCallback(std::bind(&fun, std::placeholders::_1));
	this->registerTicker(pTick, 600000, 0, 0);

	return true;
}

void CTestServiceClientApp::onDestroy()
{
}

void CTestServiceClientApp::onQuit()
{
	this->doQuit();
}

int32_t main(int argc, char* argv[])
{
	CTestServiceClientApp* pTestServiceClientApp = new CTestServiceClientApp();
	pTestServiceClientApp->run(argc, argv, "test_service_client_config.xml");

	return 0;
}