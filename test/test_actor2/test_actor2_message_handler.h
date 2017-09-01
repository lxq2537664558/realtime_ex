#pragma once
#include "libCoreCommon/service_base.h"
#include "../proto_src/request_msg3.pb.h"
#include "../proto_src/response_msg3.pb.h"
#include "libCoreCommon/actor_base.h"

using namespace core;

class CTestActor2MessageHandler
{
public:
	CTestActor2MessageHandler(CServiceBase* pServiceBase);
	~CTestActor2MessageHandler();

private:
	void request_msg3_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg3* pRequest);
};