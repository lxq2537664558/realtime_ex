#pragma once
#include <stdint.h>
#include <map>
#include <functional>
#include <string>
#include <vector>

#include "google/protobuf/message.h"

#include "libBaseNetwork/network.h"

using namespace base;

#pragma pack(push,1)
// 消息头
struct message_header
{
	uint16_t	nMessageSize;	// 包括消息头的
	uint32_t	nMessageID;
};
#pragma pack(pop)

class CMessageHandler
{
public:
	CMessageHandler();
	~CMessageHandler();

	static CMessageHandler* Inst();

	void	init(net::INetEventLoop* pNetEventLoop);

	void	dispatch(net::INetConnecterHandler* pConnection, const message_header* pData);

	void	sendMessage(net::INetConnecterHandler* pConnection, const google::protobuf::Message* pMessage);

private:
	void	registerMessageHandler(const std::string& szMessageName, const std::function<void(net::INetConnecterHandler*, const google::protobuf::Message*)>& callback);

	void	login_response_handler(net::INetConnecterHandler* pConnectToLogin, const google::protobuf::Message* pMessage);
	void	handshake_response_handler(net::INetConnecterHandler* pConnectToGate, const google::protobuf::Message* pMessage);
	void	update_name_response_handler(net::INetConnecterHandler* pConnectToGate, const google::protobuf::Message* pMessage);

private:
	struct SMessageHandler
	{
		std::string	szMessageName;
		std::function<void(net::INetConnecterHandler*, const google::protobuf::Message*)>
					callback;
	};

	std::map<uint32_t, SMessageHandler>	m_mapMessageHandler;
	std::vector<char>					m_szBuf;

	net::INetEventLoop*					m_pNetEventLoop;
};