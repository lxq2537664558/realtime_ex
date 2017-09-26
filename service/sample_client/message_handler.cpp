#include "message_handler.h"
#include "connection_to_gate.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/function_util.h"

#include "client_proto_src/c2g_player_handshake_response.pb.h"
#include "client_proto_src/c2g_player_handshake_request.pb.h"
#include "client_proto_src/c2s_update_name_request.pb.h"
#include "client_proto_src/c2s_update_name_response.pb.h"
#include "client_proto_src/c2l_player_login_response.pb.h"

#include "libBaseCommon/token_parser.h"

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

google::protobuf::Message* unserializeMessageFromBuf(const std::string& szMessageName, const void* pData, uint32_t nSize)
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

int32_t serializeMessageToBuf(const google::protobuf::Message* pMessage, void* pData, uint32_t nSize)
{
	DebugAstEx(pMessage != nullptr && pData != nullptr, -1);

	if (!pMessage->SerializeToArray(pData, nSize))
		return -1;

	return pMessage->ByteSize();
}

CMessageHandler::CMessageHandler()
{
	this->m_szBuf.resize(UINT16_MAX);
	this->m_pNetEventLoop = nullptr;
}

CMessageHandler::~CMessageHandler()
{

}

CMessageHandler* CMessageHandler::Inst()
{
	static CMessageHandler s_Inst;

	return &s_Inst;
}

void CMessageHandler::init(net::INetEventLoop* pNetEventLoop)
{
	this->m_pNetEventLoop = pNetEventLoop;

	this->registerMessageHandler("c2l_player_login_response", std::bind(&CMessageHandler::login_response_handler, this, std::placeholders::_1, std::placeholders::_2));

	this->registerMessageHandler("c2g_player_handshake_response", std::bind(&CMessageHandler::handshake_response_handler, this, std::placeholders::_1, std::placeholders::_2));
	this->registerMessageHandler("c2s_update_name_response", std::bind(&CMessageHandler::update_name_response_handler, this, std::placeholders::_1, std::placeholders::_2));
}

void CMessageHandler::dispatch(net::INetConnecterHandler* pConnection, const message_header* pData)
{
	auto iter = this->m_mapMessageHandler.find(pData->nMessageID);
	if (iter == this->m_mapMessageHandler.end())
		return;

	const char* pMessageData = reinterpret_cast<const char*>(pData + 1);
	const std::string& szMessageName = iter->second.szMessageName;

	google::protobuf::Message* pMessage = unserializeMessageFromBuf(szMessageName, pMessageData, pData->nMessageSize - sizeof(message_header));
	if (nullptr == pMessage)
	{
		PrintWarning("unserializeMessageFromBuf error message_name: {}", szMessageName);
		return;
	}

	auto& callback = iter->second.callback;
	DebugAst(callback != nullptr);

	callback(pConnection, pMessage);
}

void CMessageHandler::sendMessage(net::INetConnecterHandler* pConnection, const google::protobuf::Message* pMessage)
{
	message_header* pHeader = reinterpret_cast<message_header*>(&this->m_szBuf[0]);

	int32_t nCookiceLen = (int32_t)sizeof(message_header);

	int32_t nDataSize = serializeMessageToBuf(pMessage, &this->m_szBuf[0] + nCookiceLen, (uint32_t)(this->m_szBuf.size() - nCookiceLen));
	if (nDataSize < 0)
		return;

	pHeader->nMessageSize = uint16_t(sizeof(message_header) + nDataSize);
	pHeader->nMessageID = base::function_util::hash(pMessage->GetTypeName().c_str());

	pConnection->send(&this->m_szBuf[0], pHeader->nMessageSize, false);
}

void CMessageHandler::registerMessageHandler(const std::string& szMessageName, const std::function<void(net::INetConnecterHandler*, const google::protobuf::Message*)>& callback)
{
	DebugAst(callback != nullptr);

	uint32_t nMessageID = base::function_util::hash(szMessageName.c_str());
	auto iter = this->m_mapMessageHandler.find(nMessageID);
	if (iter != this->m_mapMessageHandler.end())
	{
		PrintWarning("dup message name message_name: {}", szMessageName);
		return;
	}

	SMessageHandler sMessageHandler;
	sMessageHandler.szMessageName = szMessageName;
	sMessageHandler.callback = callback;
	this->m_mapMessageHandler[nMessageID] = sMessageHandler;
}

void CMessageHandler::login_response_handler(net::INetConnecterHandler* pConnectToLogin, const google::protobuf::Message* pMessage)
{
	const c2l_player_login_response* pResponse = dynamic_cast<const c2l_player_login_response*>(pMessage);
	DebugAst(pResponse != nullptr);

	if (pResponse->result() != 0)
	{
		PrintWarning("");
		return;
	}

	base::CTokenParser token;
	token.parse(pResponse->gate_addr().c_str(), ":");
	uint32_t nPort = 0;
	SNetAddr netAddr;
	token.getStringElement(0, netAddr.szHost, _countof(netAddr.szHost));
	token.getUint32Element(1, nPort);
	netAddr.nPort = (uint16_t)nPort;
	
	PrintInfo("CMessageHandler::login_response_handler {}", pResponse->key());

	this->m_pNetEventLoop->connect(netAddr, 1024, 1024, new CConnectToGate(pResponse->key()));
}

void CMessageHandler::handshake_response_handler(net::INetConnecterHandler* pConnectToGate, const google::protobuf::Message* pMessage)
{
	const c2g_player_handshake_response* pResponse = dynamic_cast<const c2g_player_handshake_response*>(pMessage);
	DebugAst(pResponse != nullptr);

	c2s_update_name_request request_msg;
	request_msg.set_name("aaa");

	PrintInfo("CMessageHandler::handshake_response_handler {}", pResponse->player_id());

	this->sendMessage(pConnectToGate, &request_msg);
}

void CMessageHandler::update_name_response_handler(net::INetConnecterHandler* pConnectToGate, const google::protobuf::Message* pMessage)
{
	const c2s_update_name_response* pResponse = dynamic_cast<const c2s_update_name_response*>(pMessage);
	DebugAst(pResponse != nullptr);

	PrintInfo("update_name_response {}", pResponse->name());
}