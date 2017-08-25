#pragma once
#include "libCoreCommon/actor_message_registry.h"
#include "libCoreCommon/service_base.h"

#include "msg_proto_src/c2s_update_name_request.pb.h"

using namespace core;

class CPlayerMessageHandler :
	public CActorMessageRegistry<CPlayerMessageHandler>
{
	DEFEND_ACTOR_MESSAGE_FUNCTION(CPlayerMessageHandler)

public:
	CPlayerMessageHandler(CServiceBase* pServiceBase);
	~CPlayerMessageHandler();

private:
	void c2s_update_name_request_handler(CActorBase* pActorBase, SClientSessionInfo sClientSessionInfo, const c2s_update_name_request* pRequest);
};