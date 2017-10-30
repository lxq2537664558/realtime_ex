#include "test_service3_message_handler.h"
#include "libCoreCommon/service_invoker.h"

#include "../proto_src/request_msg2.pb.h"
#include "../proto_src/response_msg2.pb.h"
#include "libCoreCommon/register_message_util.h"


CTestService3MessageHandler::CTestService3MessageHandler(CServiceBase* pServiceBase)
{
	register_service_message_handler(pServiceBase, this, &CTestService3MessageHandler::request_msg2_handler);
}

CTestService3MessageHandler::~CTestService3MessageHandler()
{

}

void CTestService3MessageHandler::request_msg2_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const request_msg2* pRequest)
{
	response_msg2 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);
}