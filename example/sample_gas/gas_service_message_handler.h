#pragma once
#include "libCoreCommon/service_base.h"

#include "msg_proto_src/g2s_player_enter_request.pb.h"
#include "msg_proto_src/g2s_player_leave_notify.pb.h"
#include "msg_proto_src/u2s_kick_player_notify.pb.h"
#include "msg_proto_src/g2s_player_heartbeat_notify.pb.h"

using namespace core;

class CGasServiceMessageHandler
{
public:
	CGasServiceMessageHandler(CServiceBase* pServiceBase);
	~CGasServiceMessageHandler();

private:
	void g2s_player_enter_request_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_enter_request* pRequest);
	void g2s_player_leave_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_leave_notify* pMessage);
	void u2s_kick_player_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const u2s_kick_player_notify* pMessage);
	void g2s_player_heartbeat_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_heartbeat_notify* pMessage);
};