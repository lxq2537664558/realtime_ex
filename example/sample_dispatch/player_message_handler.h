#pragma once
#include "libCoreCommon/actor_message_registry.h"
#include "libCoreCommon/service_base.h"

#include "proto_src/update_name_request.pb.h"

using namespace core;

class CPlayerMessageHandler :
	public CActorMessageRegistry<CPlayerMessageHandler>
{
	DEFEND_ACTOR_MESSAGE_FUNCTION(CPlayerMessageHandler)

public:
	CPlayerMessageHandler(CServiceBase* pServiceBase);
	~CPlayerMessageHandler();

private:
	void update_name_handler(CActorBase* pActorBase, SClientSessionInfo sClientSessionInfo, const update_name_request* pRequest);
};