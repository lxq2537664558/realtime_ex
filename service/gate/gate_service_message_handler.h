#pragma once
#include "libCoreCommon/service_message_registry.h"
#include "libCoreCommon/service_base.h"

#include "proto_src/player_token_notify.pb.h"

class CGateServiceMessageHandler :
	public core::CServiceMessageRegistry<CGateServiceMessageHandler>
{
	DEFEND_SERVICE_MESSAGE_FUNCTION(CGateServiceMessageHandler)

public:
	CGateServiceMessageHandler(core::CServiceBase* pServiceBase);
	~CGateServiceMessageHandler();

private:
	void		player_token_handler(core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const player_token_notify* pRequest);
};