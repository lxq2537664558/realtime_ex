#pragma once
#include "libCoreCommon/service_message_registry.h"
#include "libCoreCommon/service_base.h"
#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"

using namespace core;

class CTestService2MessageHandler :
	public CServiceMessageRegistry<CTestService2MessageHandler>
{
	DEFEND_SERVICE_MESSAGE_FUNCTION(CTestService2MessageHandler)

public:
	CTestService2MessageHandler(CServiceBase* pServiceBase);
	~CTestService2MessageHandler();

private:
	bool request_msg1_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const request_msg1* pRequest);
};