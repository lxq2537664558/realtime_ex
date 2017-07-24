#pragma once
#include "libCoreCommon\service_message_registry.h"
#include "libCoreCommon\service_base.h"
#include "../proto_src/request_msg2.pb.h"
#include "../proto_src/response_msg2.pb.h"

using namespace core;

class CTestService3MessageHandler :
	public CServiceMessageRegistry<CTestService3MessageHandler>
{
	DEFEND_SERVICE_MESSAGE_FUNCTION(CTestService3MessageHandler)

public:
	CTestService3MessageHandler(CServiceBase* pServiceBase);
	~CTestService3MessageHandler();

private:
	bool request_msg2_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const request_msg2* pRequest);
};