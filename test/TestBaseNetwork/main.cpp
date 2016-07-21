#include "libBaseNetwork/network.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"

#include <iostream>
#include "libBaseCommon/debug_helper.h"
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

struct SNetConnecterHandler :
	public base::INetConnecterHandler
{
	uint32_t g_nTotalSend = 0;
	uint32_t g_nTotalRecv = 0;

	virtual uint32_t onRecv(const char* pData, uint32_t nDataSize)
	{
		uint32_t nRecvSize = 0;
		do
		{
			uint16_t nMessageSize = 0;

			// 都不够消息头
			if (nDataSize < sizeof(message_header))
				break;

			const message_header* pHeader = reinterpret_cast<const message_header*>(pData);
			if (pHeader->nMessageSize < sizeof(message_header))
			{
				this->m_pNetConnecter->shutdown(true, "");
				break;
			}

			// 不是完整的消息
			if (nDataSize < pHeader->nMessageSize)
				break;

			nMessageSize = pHeader->nMessageSize;

			this->onDispatch(pHeader);

			nRecvSize += nMessageSize;
			nDataSize -= nMessageSize;
			pData += nMessageSize;

		} while (true);

		return nRecvSize;
	}

	void onDispatch(const message_header* pHeader)
	{
		DebugAst(pHeader->nMessageID == _MESSAGE_ID);
		const STestMsg* pTestMsg = reinterpret_cast<const STestMsg*>(pHeader);
		DebugAst(pTestMsg->nCount == g_nTotalRecv);
		++g_nTotalRecv;
		
		//PrintDebug("client delta time: %d server delta time: %d count: %d", (uint32_t)(base::getGmtTime() - pTestMsg->nClientTime), (uint32_t)(base::getGmtTime() - pTestMsg->nClientTime), pTestMsg->nCount);
		this->sendMsg();
	}

	virtual void   onConnect() override
	{
		PrintDebug("onConnect");
		this->sendMsg();
	}

	virtual void   onDisconnect() override
	{
		PrintDebug("onDisconnect");
	}

	void sendMsg()
	{
		STestMsg netMsg;
		netMsg.nMessageID = _MESSAGE_ID;
		netMsg.nClientTime = base::getGmtTime();
		netMsg.nCount = g_nTotalSend++;
		netMsg.nMessageSize = sizeof(netMsg);

		this->m_pNetConnecter->send(&netMsg, sizeof(netMsg));
	}
};

struct SNetAccepterHandler :
	public base::INetAccepterHandler
{
	virtual base::INetConnecterHandler* onAccept(base::INetConnecter* pNetConnecter)
	{
		return new SNetConnecterHandler();
	}
};

int main()
{
	base::initLog(true, "./log");
	base::startupNetwork();
	
	SNetAccepterHandler* pNetAccepterHandler = new SNetAccepterHandler();
	base::INetEventLoop* pNetEventLoop = base::createNetEventLoop();
	pNetEventLoop->init(50000);

#ifdef _TEST_LISTEN
	SNetAddr netAddr;
	netAddr.nPort = 8000;
	base::crt::strcpy(netAddr.szHost, _countof(netAddr.szHost), "0.0.0.0");
	pNetEventLoop->listen(netAddr, 10 * 1024, 10 * 1024, pNetAccepterHandler);
#else
	SNetAddr netAddr;
	netAddr.nPort = 12000;
	base::crt::strcpy(netAddr.szHost, _countof(netAddr.szHost), "192.168.222.131");
	for (size_t i = 0; i < 300; ++i)
	{
		pNetEventLoop->connect(netAddr, 1024, 1024, new SNetConnecterHandler());
		base::sleep(100);
	}
#endif

	while( true )
	{
		pNetEventLoop->update(10 );
	}

	return 0;
}