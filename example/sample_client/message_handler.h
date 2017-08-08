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
// ��Ϣͷ
struct message_header
{
	uint16_t	nMessageSize;	// ������Ϣͷ��
	uint32_t	nMessageID;
};
#pragma pack(pop)

class CMessageHandler
{
public:
	CMessageHandler();
	~CMessageHandler();

	static CMessageHandler* Inst();

	void	init(INetEventLoop* pNetEventLoop);

	void	dispatch(INetConnecterHandler* pConnection, const message_header* pData);

	void	sendMessage(INetConnecterHandler* pConnection, const google::protobuf::Message* pMessage);

private:
	void	registerMessageHandler(const std::string& szMessageName, const std::function<void(INetConnecterHandler*, const google::protobuf::Message*)>& callback);

	void	login_response_handler(INetConnecterHandler* pConnectToLogin, const google::protobuf::Message* pMessage);
	void	handshake_response_handler(INetConnecterHandler* pConnectToGate, const google::protobuf::Message* pMessage);
	void	update_name_response_handler(INetConnecterHandler* pConnectToGate, const google::protobuf::Message* pMessage);

private:
	struct SMessageHandler
	{
		std::string	szMessageName;
		std::function<void(INetConnecterHandler*, const google::protobuf::Message*)>
					callback;
	};

	std::map<uint32_t, SMessageHandler>	m_mapMessageHandler;
	std::vector<char>					m_szBuf;

	INetEventLoop*						m_pNetEventLoop;
};