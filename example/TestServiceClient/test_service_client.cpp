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
#include "../proto_src/login_msg.pb.h"

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

google::protobuf::Message* create_protobuf_message(const std::string& szMessageName)
{
	const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szMessageName);
	if (pDescriptor == nullptr)
		return nullptr;

	const google::protobuf::Message* pProtoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
	if (pProtoType == nullptr)
		return nullptr;

	return pProtoType->New();
}

google::protobuf::Message* unserialize_protobuf_message_from_buf(const std::string& szMessageName, const message_header* pHeader)
{
	google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
	if (nullptr == pMessage)
		return nullptr;

	if (!pMessage->ParseFromArray(pHeader + 1, pHeader->nMessageSize - sizeof(message_header)))
	{
		SAFE_DELETE(pMessage);
		return nullptr;
	}

	return pMessage;
}

int32_t serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, message_header* pHeader, uint32_t nSize)
{
	DebugAstEx(pMessage != nullptr, false);

	std::string szMessageData;
	if (!pMessage->SerializeToString(&szMessageData))
		return -1;

	if (szMessageData.size() > nSize)
		return -1;

	pHeader->nMessageID = base::hash(pMessage->GetTypeName().c_str());
	pHeader->nMessageSize = (uint16_t)(sizeof(message_header) + szMessageData.size());

	memcpy(pHeader + 1, szMessageData.c_str(), szMessageData.size());

	return (int32_t)(szMessageData.size() + sizeof(message_header));
}

struct SNetConnecterHandler :
	public base::INetConnecterHandler
{
	SNetConnecterHandler()
	{
		nNextPacketID = 0;
	}

	virtual uint32_t onRecv(const char* pData, uint32_t nDataSize) override
	{
		uint64_t nRecvTime = base::getGmtTime();
		google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf("test.client_response_msg", reinterpret_cast<const message_header*>(pData));
		test::client_response_msg* pMsg = dynamic_cast<test::client_response_msg*>(pMessage);
		PrintDebug("onRecv %s %d %d %d", pMsg->name().c_str(), nNextPacketID, pMsg->id(), (uint32_t)(nRecvTime - nSendTime));
		this->requestMsg("aa");
		return nDataSize;
	}

	virtual void   onConnect() override
	{
		PrintDebug("onConnect");

		this->login(nID);

		this->requestMsg("aa");
	}

	void login(uint64_t nID)
	{
		PrintDebug("Login "UINT64FMT, nID);
		gate::login_msg msg;
		msg.set_id(nID);
		std::string szMessageData;
		msg.SerializeToString(&szMessageData);

		message_header header;
		header.nMessageSize = (uint16_t)(sizeof(header) + szMessageData.size());
		header.nMessageID = base::hash(msg.GetTypeName().c_str());

		this->m_pNetConnecter->send(&header, sizeof(header));
		this->m_pNetConnecter->send(szMessageData.c_str(), (uint16_t)szMessageData.size());
	}

	void requestMsg(const std::string& szName)
	{
		test::client_request_msg msg;
		msg.set_name(szName);
		msg.set_id(nNextPacketID);
		std::string szMessageData;
		msg.SerializeToString(&szMessageData);

		message_header header;
		header.nMessageSize = (uint16_t)(sizeof(header) + szMessageData.size());
		header.nMessageID = base::hash(msg.GetTypeName().c_str());

		this->m_pNetConnecter->send(&header, sizeof(header));
		this->m_pNetConnecter->send(szMessageData.c_str(), (uint16_t)szMessageData.size());

		nSendTime = base::getGmtTime();
		++nNextPacketID;
	}

	virtual void   onDisconnect() override
	{
		PrintDebug("onDisconnect");
	}

public:
	uint64_t	nID;
	uint32_t	nNextPacketID;
	uint64_t	nSendTime;
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
	for (size_t i = 0; i < 1; ++i)
	{
		SNetConnecterHandler* pNetConnecterHandler = new SNetConnecterHandler();
		pNetConnecterHandler->nID = 10000 + i;
		pNetEventLoop->connect(sNedAddr, 1024, 1024, pNetConnecterHandler);
	}
	while (true)
	{
		pNetEventLoop->update(100);
	}

	return 0;
}