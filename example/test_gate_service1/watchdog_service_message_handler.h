#pragma once
#include "libCoreCommon\service_message_registry.h"
#include "libCoreCommon\service_base.h"

#include "..\..\service\gate\proto_src\player_enter_request.pb.h"
#include "..\..\service\gate\proto_src\player_leave.pb.h"

using namespace core;

class CWatchdogServiceMessageHandler :
	public CServiceMessageRegistry<CWatchdogServiceMessageHandler>
{
	DEFEND_SERVICE_MESSAGE_FUNCTION(CWatchdogServiceMessageHandler)

public:
	CWatchdogServiceMessageHandler(CServiceBase* pServiceBase);
	~CWatchdogServiceMessageHandler();

private:
	void player_enter_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const player_enter_request* pRequest);
	void player_leave_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const player_leave* pMessage);
};