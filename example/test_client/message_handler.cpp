#include "stdafx.h"
#include "message_handler.h"
#include "libBaseCommon\debug_helper.h"
#include "libBaseCommon\base_function.h"
#include "connection_to_gate.h"

#include "..\..\service\gate\proto_src\gate_handshake_response.pb.h"
#include "..\test_gate_service1\proto_src\update_name_request.pb.h"
#include "..\test_gate_service1\proto_src\update_name_response.pb.h"

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

google::protobuf::Message* unserialize_protobuf_message_from_buf(const std::string& szMessageName, const void* pData, uint32_t nSize)
{
	DebugAstEx(pData != nullptr, nullptr);

	google::protobuf::Message* pMessage = create_protobuf_message(szMessageName);
	if (nullptr == pMessage)
		return nullptr;

	if (!pMessage->ParseFromArray(pData, nSize))
	{
		SAFE_DELETE(pMessage);
		return nullptr;
	}

	return pMessage;
}

int32_t serialize_protobuf_message_to_buf(const google::protobuf::Message* pMessage, void* pData, uint32_t nSize)
{
	DebugAstEx(pMessage != nullptr && pData != nullptr, -1);

	if (!pMessage->SerializeToArray(pData, nSize))
		return -1;

	return pMessage->ByteSize();
}

CMessageHandler::CMessageHandler()
{
	this->m_szBuf.resize(UINT16_MAX);
}

CMessageHandler::~CMessageHandler()
{

}

CMessageHandler* CMessageHandler::Inst()
{
	static CMessageHandler s_Inst;

	return &s_Inst;
}

void CMessageHandler::init()
{
	this->registerMessageHandler("gate_handshake_response", std::bind(&CMessageHandler::handshake_response_handler, this, std::placeholders::_1, std::placeholders::_2));
	this->registerMessageHandler("update_name_response", std::bind(&CMessageHandler::update_name_response_handler, this, std::placeholders::_1, std::placeholders::_2));
}

void CMessageHandler::dispatch(CConnectToGate* pConnectToGate, const message_header* pData)
{
	auto iter = this->m_mapMessageHandler.find(pData->nMessageID);
	if (iter == this->m_mapMessageHandler.end())
		return;

	const char* pMessageData = reinterpret_cast<const char*>(pData + 1);
	const std::string& szMessageName = iter->second.szMessageName;

	google::protobuf::Message* pMessage = unserialize_protobuf_message_from_buf(szMessageName, pMessageData, pData->nMessageSize - sizeof(message_header));
	if (nullptr == pMessage)
	{
		PrintWarning("unserialize_protobuf_message_from_buf error message_name: %s", szMessageName.c_str());
		return;
	}

	auto& callback = iter->second.callback;
	DebugAst(callback != nullptr);

	callback(pConnectToGate, pMessage);
}

void CMessageHandler::sendMessage(CConnectToGate* pConnectToGate, const google::protobuf::Message* pMessage)
{
	message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

	int32_t nCookiceLen = (int32_t)sizeof(message_header);

	int32_t nDataSize = serialize_protobuf_message_to_buf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
	if (nDataSize < 0)
		return;

	pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
	pHeader->nMessageID = base::hash(pMessage->GetTypeName().c_str());

	pConnectToGate->send(&this->m_szBuf[0], pHeader->nMessageSize, false);
}

void CMessageHandler::registerMessageHandler(const std::string& szMessageName, const std::function<void(CConnectToGate*, const google::protobuf::Message*)>& callback)
{
	DebugAst(callback != nullptr);

	uint32_t nMessageID = base::hash(szMessageName.c_str());
	auto iter = this->m_mapMessageHandler.find(nMessageID);
	if (iter != this->m_mapMessageHandler.end())
	{
		PrintWarning("dup message name message_name: %s", szMessageName.c_str());
		return;
	}

	SMessageHandler sMessageHandler;
	sMessageHandler.szMessageName = szMessageName;
	sMessageHandler.callback = callback;
	this->m_mapMessageHandler[nMessageID] = sMessageHandler;
}

void CMessageHandler::handshake_response_handler(CConnectToGate* pConnectToGate, const google::protobuf::Message* pMessage)
{
	const gate_handshake_response* pResponse = dynamic_cast<const gate_handshake_response*>(pMessage);
	DebugAst(pResponse != nullptr);

	update_name_request request_msg;
	request_msg.set_name("aaa");

	this->sendMessage(pConnectToGate, &request_msg);
}

void CMessageHandler::update_name_response_handler(CConnectToGate* pConnectToGate, const google::protobuf::Message* pMessage)
{
	const update_name_response* pResponse = dynamic_cast<const update_name_response*>(pMessage);
	DebugAst(pResponse != nullptr);

	PrintInfo("update_name_response %s", pResponse->name().c_str());
}