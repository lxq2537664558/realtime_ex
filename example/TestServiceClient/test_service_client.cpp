// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_client.h"

#include "libBaseNetwork/network.h"
#include "libBaseCommon/thread_base.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/logger.h"
#include "libBaseCommon/base_function.h"

#include "../proto_src/client_request_msg.pb.h"
#include "../proto_src/client_response_msg.pb.h"

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

struct SNetConnecterHandler :
	public base::INetConnecterHandler
{
	virtual uint32_t onRecv(const char* pData, uint32_t nDataSize) override
	{
		return nDataSize;
	}

	virtual void   onConnect() override
	{
		PrintDebug("onConnect");
		test::client_request_msg msg;
		msg.set_name("aaaaa");
		std::string szMessageData;
		msg.SerializeToString(&szMessageData);

		message_header header;
		header.nMessageSize = (uint16_t)(sizeof(header) + szMessageData.size());
		header.nMessageID = base::hash(msg.GetTypeName().c_str());

		this->m_pNetConnecter->send(&header, sizeof(header));
		this->m_pNetConnecter->send(szMessageData.c_str(), (uint16_t)szMessageData.size());
	}

	virtual void   onDisconnect() override
	{
		PrintDebug("onDisconnect");
	}
};

int32_t main(int argc, char* argv[])
{
	base::initLog(true);
	base::startupNetwork();

	base::INetEventLoop* pNetEventLoop = base::createNetEventLoop();
	pNetEventLoop->init(50000);
	SNetAddr sNedAddr;
	sNedAddr.nPort = 8000;
	strncpy_s(sNedAddr.szHost, "127.0.0.1", _countof(sNedAddr.szHost));
	SNetConnecterHandler* pNetConnecterHandler = new SNetConnecterHandler();
	pNetEventLoop->connect(sNedAddr, 1024, 1024, pNetConnecterHandler);
	while (true)
	{
		pNetEventLoop->update(100);
	}
	
	SAFE_DELETE(pNetConnecterHandler);

	return 0;
}