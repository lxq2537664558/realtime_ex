#pragma once
#include "libCoreCommon/service_base.h"

#include "msg_proto_src/d2g_player_token_notify.pb.h"
#include "msg_proto_src/s2g_kick_player_notify.pb.h"

class CGateServiceMessageHandler
{
public:
	CGateServiceMessageHandler(core::CServiceBase* pServiceBase);
	~CGateServiceMessageHandler();

private:
	void		d2g_player_token_notify_handler(core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const d2g_player_token_notify* pRequest);
	void		s2g_kick_player_notify_handler(core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2g_kick_player_notify* pRequest);
};