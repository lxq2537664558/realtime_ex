#pragma once

#include "libCoreCommon/core_common.h"

#include "login_connection_from_client.h"

class CLoginService;
class CLoginClientMessageHandler
{
public:
	CLoginClientMessageHandler(CLoginService* pLoginService);
	~CLoginClientMessageHandler();

	void	sendClientMessage(core::CBaseConnection* pBaseConnection, const google::protobuf::Message* pMessage);

private:
	void	login(CLoginConnectionFromClient* pConnectionFromClient, const google::protobuf::Message* pMessage);

private:
	CLoginService*		m_pLoginService;
	std::vector<char>	m_szBuf;
};