#include "libBaseCommon/base_time.h"
#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/ticker.h"
#include "libCoreCommon/coroutine.h"

#include <map>

#include <stdio.h>
#include "libBaseCommon/rand_gen.h"
#include "libCoreCommon/base_connection_factory.h"

class CTestApp;
CTestApp* g_pApp;

void fun1(uint64_t nContext)
{
	while (true)
	{
		core::coroutine::yield();
		PrintDebug("111111111 %d", (uint32_t)nContext);
	}
}

void fun2(uint64_t nContext)
{
	PrintDebug("fun2");
}

struct SComp
{
	bool operator () (uint64_t lhs, uint64_t rhs)
	{
		return lhs < rhs;
	}
};

#include <iostream>
#include <queue>
using namespace std;

#pragma pack(push,1)
// 消息头
struct message_header
{
	uint16_t	nMessageSize;	// 包括消息头的
	uint16_t	nMessageID;
};

struct STestMsg : public message_header
{
	uint64_t nClientTime;
	uint64_t nServerTime;
	uint32_t nCount;
};
#pragma pack(pop)

#define _MESSAGE_ID 1000
#define _TEST_CONNECTION 1

class CTestConnection : 
	public core::CBaseConnection
{
public:
	virtual void		onConnect()
	{

	}
	
	virtual void		onDisconnect()
	{
		PrintDebug("onDisconnect");
	}

	virtual uint32_t	getType() const
	{
		return _TEST_CONNECTION;
	}

	virtual void		release()
	{
		delete this;
	}
	
	virtual bool	onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		const STestMsg* pTestMsg = reinterpret_cast<const STestMsg*>(pData);
		const_cast<STestMsg*>(pTestMsg)->nServerTime = base::getGmtTime();
		//PrintDebug("delta time: %d", (uint32_t)(pTestMsg->nServerTime - pTestMsg->nClientTime));
		this->send(eMT_CLIENT, pData, nSize);

		return true;
	}
};


class CTestBaseConnectionFactory :
	public core::CBaseConnectionFactory
{
public:
	CTestBaseConnectionFactory() { }
	virtual ~CTestBaseConnectionFactory() { }

	virtual core::CBaseConnection*	createBaseConnection(uint32_t nType, const std::string& szContext)
	{
		DebugAstEx(nType == 1, nullptr);

		CTestConnection* pBaseConnection = new CTestConnection();
		pBaseConnection->init(szContext);

		return pBaseConnection;
	}
};

class CTestApp :
	public core::CBaseApp
{
public:
	virtual bool onInit()
	{	
		core::CTicker* pTicker = new core::CTicker();
		pTicker->setCallback(std::bind(&fun2, std::placeholders::_1));

		this->registerTicker(pTicker, 1000, 1000, 0);
		this->getBaseConnectionMgr()->setBaseConnectionFactory(_TEST_CONNECTION, new CTestBaseConnectionFactory());
		this->getBaseConnectionMgr()->listen("0.0.0.0", 8000, 1, "", 1024, 1024, default_client_message_parser);
		return true;
	}

	virtual void onQuit()
	{
		
	}
};

int32_t main(int32_t argc, char* argv[])
{
	g_pApp = new CTestApp();
	g_pApp->run(argc, argv, "gate_config.xml");

	return 0;
}