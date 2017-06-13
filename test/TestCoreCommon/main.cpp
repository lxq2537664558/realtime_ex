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

void fun2(uint64_t nContext);

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
// ��Ϣͷ
struct message_header
{
	uint16_t	nMessageSize;	// ������Ϣͷ��
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
	
	virtual void onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		const STestMsg* pTestMsg = reinterpret_cast<const STestMsg*>(pData);
		const_cast<STestMsg*>(pTestMsg)->nServerTime = base::getGmtTime();
		//PrintDebug("delta time: %d", (uint32_t)(pTestMsg->nServerTime - pTestMsg->nClientTime));
		this->send(eMT_CLIENT, pData, nSize);
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
		{
			base::STime sGmtTime = base::getGmtTimeTM();
			char szGmtTime[30] = { 0 };
			base::crt::snprintf(szGmtTime, _countof(szGmtTime), "%04d-%02d-%02d %02d:%02d:%02d",
				sGmtTime.nYear, sGmtTime.nMon, sGmtTime.nDay, sGmtTime.nHour, sGmtTime.nMin, sGmtTime.nSec);
			base::STime sLogicTime = base::getGmtTimeTM(g_pApp->getLogicTime());
			char szLogicTime[30] = { 0 };
			base::crt::snprintf(szLogicTime, _countof(szLogicTime), "%04d-%02d-%02d %02d:%02d:%02d",
				sLogicTime.nYear, sLogicTime.nMon, sLogicTime.nDay, sLogicTime.nHour, sLogicTime.nMin, sLogicTime.nSec);
			PrintDebug("onInit11111111111 %s %s", szGmtTime, szLogicTime);
		}
		Sleep(10000);
		core::CTicker* pTicker = new core::CTicker();
		pTicker->setCallback(std::bind(&fun2, std::placeholders::_1));
		{
			base::STime sGmtTime = base::getGmtTimeTM();
			char szGmtTime[30] = { 0 };
			base::crt::snprintf(szGmtTime, _countof(szGmtTime), "%04d-%02d-%02d %02d:%02d:%02d",
				sGmtTime.nYear, sGmtTime.nMon, sGmtTime.nDay, sGmtTime.nHour, sGmtTime.nMin, sGmtTime.nSec);
			base::STime sLogicTime = base::getGmtTimeTM(g_pApp->getLogicTime());
			char szLogicTime[30] = { 0 };
			base::crt::snprintf(szLogicTime, _countof(szLogicTime), "%04d-%02d-%02d %02d:%02d:%02d",
				sLogicTime.nYear, sLogicTime.nMon, sLogicTime.nDay, sLogicTime.nHour, sLogicTime.nMin, sLogicTime.nSec);
			PrintDebug("onInit2222222222 %s %s", szGmtTime, szLogicTime);
		}
		this->registerTicker(pTicker, 20000, 0, 0);
		this->getBaseConnectionMgr()->setBaseConnectionFactory(_TEST_CONNECTION, new CTestBaseConnectionFactory());
		this->getBaseConnectionMgr()->listen("0.0.0.0", 8000, 1, "", 1024, 1024);
		return true;
	}

	virtual void onQuit()
	{
		
	}
};

void fun2(uint64_t nContext)
{
	base::STime sGmtTime = base::getGmtTimeTM();
	char szGmtTime[30] = { 0 };
	base::crt::snprintf(szGmtTime, _countof(szGmtTime), "%04d-%02d-%02d %02d:%02d:%02d",
		sGmtTime.nYear, sGmtTime.nMon, sGmtTime.nDay, sGmtTime.nHour, sGmtTime.nMin, sGmtTime.nSec);
	base::STime sLogicTime = base::getGmtTimeTM(g_pApp->getLogicTime());
	char szLogicTime[30] = { 0 };
	base::crt::snprintf(szLogicTime, _countof(szLogicTime), "%04d-%02d-%02d %02d:%02d:%02d",
		sLogicTime.nYear, sLogicTime.nMon, sLogicTime.nDay, sLogicTime.nHour, sLogicTime.nMin, sLogicTime.nSec);
	PrintDebug("aaaaaaaaaafun2 %s %s", szGmtTime, szLogicTime);
}

std::string fun(int a)
{
	static char* s_szValue = "aaa";
	string aa = "aa";
	string bb = "bb";

	if (a == 0)
		return aa;
	else
		return bb;
}

int32_t main(int32_t argc, char* argv[])
{
	std::string szValue = fun(0);

	g_pApp = new CTestApp();
	g_pApp->run(argc, argv, "gate_config.xml");

	return 0;
}