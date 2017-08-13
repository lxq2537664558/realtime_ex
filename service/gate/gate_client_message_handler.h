#pragma once

#include "libCoreCommon/core_common.h"

#include "gate_connection_from_client.h"

class CGateService;
class CGateClientMessageHandler
{
public:
	CGateClientMessageHandler(CGateService*	pGateService);
	~CGateClientMessageHandler();

	void	sendClientMessage(core::CBaseConnection* pBaseConnection, const google::protobuf::Message* pMessage);

private:
	void	handshake(CGateConnectionFromClient* pGateConnectionFromClient, const google::protobuf::Message* pMessage);

private:
	CGateService*		m_pGateService;
	std::vector<char>	m_szBuf;
};