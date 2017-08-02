#pragma once

#include "libCoreCommon/core_common.h"

#include "connection_from_client.h"
#include "proto_src/player_enter_response.pb.h"

class CGateService;
class CClientMessageHandler
{
public:
	CClientMessageHandler(CGateService*	pGateService);
	~CClientMessageHandler();

	void	sendClientMessage(core::CBaseConnection* pBaseConnection, const google::protobuf::Message* pMessage);

private:
	void	handshake(CConnectionFromClient* pConnectionFromClient, const google::protobuf::Message* pMessage);

private:
	CGateService*		m_pGateService;
	std::vector<char>	m_szBuf;
};