#pragma once
#include "libCoreCommon/service_message_registry.h"
#include "libCoreCommon/service_base.h"

#include "proto_src/player_enter_gas_request.pb.h"
#include "proto_src/player_leave_gas_notify.pb.h"

using namespace core;

class CWatchdogServiceMessageHandler :
	public CServiceMessageRegistry<CWatchdogServiceMessageHandler>
{
	DEFEND_SERVICE_MESSAGE_FUNCTION(CWatchdogServiceMessageHandler)

public:
	CWatchdogServiceMessageHandler(CServiceBase* pServiceBase);
	~CWatchdogServiceMessageHandler();

private:
	void player_enter_gas_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const player_enter_gas_request* pRequest);
	void player_leave_gas_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const player_leave_gas_notify* pMessage);
};