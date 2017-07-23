#pragma once
#include "libCoreCommon/actor_message_registry.h"
#include "libCoreCommon/service_base.h"
#include "../proto_src/request_msg3.pb.h"
#include "../proto_src/response_msg3.pb.h"

using namespace core;

class CTestActor2MessageHandler :
	public CActorMessageRegistry<CTestActor2MessageHandler>
{
	DEFEND_ACTOR_MESSAGE_FUNCTION(CTestActor2MessageHandler)

public:
	CTestActor2MessageHandler(CServiceBase* pServiceBase);
	~CTestActor2MessageHandler();

	bool init();

	bool request_msg3_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg3* pRequest);

private:
	CServiceBase* m_pServiceBase;
};