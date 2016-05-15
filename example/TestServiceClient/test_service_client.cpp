// TestServiceA.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "test_service_client.h"

#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/base_connection.h"
#include "libBaseCommon/base_time.h"

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

class CConnectToService :
	public core::CBaseConnection
{
	DECLARE_OBJECT(CConnectToService)

public:
	CConnectToService()
	{
		nNextPacketID = 0;
	}

	virtual ~CConnectToService() 
	{
	}
	
	virtual void onConnect(const std::string& szContext)
	{
		PrintDebug("onConnect");

		this->login(nID);

		this->requestMsg("aa");
	}
	
	virtual void onDisconnect()
	{
		PrintDebug("onDisconnect");
	}

	virtual void onDispatch(uint32_t nMessageType, const void* pData, uint16_t nSize)
	{
		google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf("test.client_response_msg", reinterpret_cast<const message_header*>(pData));
		test::client_response_msg* pMsg = dynamic_cast<test::client_response_msg*>(pMessage);
		
	}

private:
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

		this->send(eMT_CLIENT, &header, sizeof(header), szMessageData.c_str(), (uint16_t)szMessageData.size());
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

		this->send(eMT_CLIENT, &header, sizeof(header), szMessageData.c_str(), (uint16_t)szMessageData.size());

		nSendTime = base::getGmtTime();
		++nNextPacketID;
	}

public:
	uint64_t	nID;
	uint32_t	nNextPacketID;
	uint64_t	nSendTime;
};

DEFINE_OBJECT(CConnectToService, 100)

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
	CConnectToService::registClassInfo();
	this->getBaseConnectionMgr()->connect("127.0.0.1", 8000, "", _GET_CLASS_NAME(CConnectToService), 0, 0, core::default_parser_native_data);
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