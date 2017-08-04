#pragma once
#include <stdint.h>
#include <map>
#include <functional>
#include <string>
#include <vector>

#include "google\protobuf\message.h"

#pragma pack(push,1)
// 消息头
struct message_header
{
	uint16_t	nMessageSize;	// 包括消息头的
	uint32_t	nMessageID;
};
#pragma pack(pop)

class CConnectToGate;
class CMessageHandler
{
public:
	CMessageHandler();
	~CMessageHandler();

	static CMessageHandler* Inst();

	void	init();

	void	dispatch(CConnectToGate* pConnectToGate, const message_header* pData);

	void	sendMessage(CConnectToGate* pConnectToGate, const google::protobuf::Message* pMessage);

private:
	void	registerMessageHandler(const std::string& szMessageName, const std::function<void(CConnectToGate*, const google::protobuf::Message*)>& callback);

	void	handshake_response_handler(CConnectToGate* pConnectToGate, const google::protobuf::Message* pMessage);
	void	update_name_response_handler(CConnectToGate* pConnectToGate, const google::protobuf::Message* pMessage);

private:
	struct SMessageHandler
	{
		std::string	szMessageName;
		std::function<void(CConnectToGate*, const google::protobuf::Message*)>
					callback;
	};

	std::map<uint32_t, SMessageHandler>	m_mapMessageHandler;
	std::vector<char>					m_szBuf;
};