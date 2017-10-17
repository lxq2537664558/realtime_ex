#pragma once
#include "libCoreCommon/service_base.h"
#include "../proto_src/request_msg2.pb.h"
#include "../proto_src/response_msg2.pb.h"

using namespace core;

class CTestService3MessageHandler
{
public:
	CTestService3MessageHandler(CServiceBase* pServiceBase);
	~CTestService3MessageHandler();

private:
	void request_msg2_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const request_msg2* pRequest);
};