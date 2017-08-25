#pragma once
#include "libCoreCommon/service_message_registry.h"
#include "libCoreCommon/service_base.h"

#include "msg_proto_src/d2u_player_login_request.pb.h"
#include "msg_proto_src/s2u_player_heartbeat_notify.pb.h"
#include "msg_proto_src/s2u_player_leave_notify.pb.h"

using namespace core;

class CUCServiceMessageHandler :
	public CServiceMessageRegistry<CUCServiceMessageHandler>
{
	DEFEND_SERVICE_MESSAGE_FUNCTION(CUCServiceMessageHandler)

public:
	CUCServiceMessageHandler(CServiceBase* pServiceBase);
	~CUCServiceMessageHandler();

private:
	void	d2u_player_login_request_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const d2u_player_login_request* pRequest);
	void	s2u_player_heartbeat_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2u_player_heartbeat_notify* pRequest);
	void	s2u_player_leave_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2u_player_leave_notify* pRequest);
};
