#pragma once
#include "libCoreCommon/service_base.h"
#include "../proto_src/request_msg0.pb.h"
#include "../proto_src/response_msg0.pb.h"
#include "libCoreCommon/actor_base.h"

using namespace core;

class CTestActor0MessageHandler
{
public:
	CTestActor0MessageHandler(CServiceBase* pServiceBase);
	~CTestActor0MessageHandler();

private:
	void request_msg0_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg0* pRequest);
};