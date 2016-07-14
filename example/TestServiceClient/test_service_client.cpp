// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_client.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libBaseCommon/base_time.h"

#include "libCoreCommon/base_connection_factory.h"
#include "../common/test_message_define.h"

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

		this->requestMsg(1);
	}
	
	virtual void onDisconnect()
	{
		PrintDebug("onDisconnect");
	}

	virtual bool onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		const SClientResponseMsg* pMsg = static_cast<const SClientResponseMsg*>(pData);
		PrintInfo("onDispatch %d", pMsg->nID);

		return true;
	}

private:
	void requestMsg(uint32_t nID)
	{
		SClientRequestMsg msg;
		msg.nID = nID;

		this->send(eMT_CLIENT, &msg, sizeof(msg));

		nSendTime = base::getGmtTime();
		++nNextPacketID;
	}

public:
	uint64_t	nID;
	uint32_t	nNextPacketID;
	uint64_t	nSendTime;
};

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

bool CTestServiceClientApp::onInit()
{
	CServiceConnectionFactory* pServiceConnectionFactory = new CServiceConnectionFactory();
	this->getBaseConnectionMgr()->setBaseConnectionFactory(_BASE_CONNECTION_TYPE_BEGIN, pServiceConnectionFactory);
	this->getBaseConnectionMgr()->connect("127.0.0.1", 8000, _BASE_CONNECTION_TYPE_BEGIN, "", 0, 0, default_client_message_parser);
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