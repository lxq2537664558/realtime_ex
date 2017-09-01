#include "test_service2_message_handler.h"
#include "libCoreCommon/service_invoker.h"

#include "../proto_src/request_msg1.pb.h"
#include "../proto_src/response_msg1.pb.h"


CTestService2MessageHandler::CTestService2MessageHandler(CServiceBase* pServiceBase)
{
	register_service_message_handler(pServiceBase, this, &CTestService2MessageHandler::request_msg1_handler);
}

CTestService2MessageHandler::~CTestService2MessageHandler()
{

}

void CTestService2MessageHandler::request_msg1_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const request_msg1* pRequest)
{
	response_msg1 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);
}