#pragma once
#include "libCoreCommon/actor_message_registry.h"
#include "libCoreCommon/service_base.h"
#include "../proto_src/request_msg0.pb.h"
#include "../proto_src/response_msg0.pb.h"

using namespace core;

class CTestActor0MessageHandler :
	public CActorMessageRegistry<CTestActor0MessageHandler>
{
	DEFEND_ACTOR_MESSAGE_FUNCTION(CTestActor0MessageHandler)

public:
	CTestActor0MessageHandler(CServiceBase* pServiceBase);
	~CTestActor0MessageHandler();

	bool init();

	bool request_msg0_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg0* pRequest);

private:
	CServiceBase* m_pServiceBase;
};