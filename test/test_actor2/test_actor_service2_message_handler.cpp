#include "test_actor_service2_message_handler.h"

#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/service_base.h"

CTestActorService2MessageHandler::CTestActorService2MessageHandler(CServiceBase* pServiceBase)
{
	register_service_message_handler(pServiceBase, this, &CTestActorService2MessageHandler::request_msg2_handler);
}

CTestActorService2MessageHandler::~CTestActorService2MessageHandler()
{

}

void CTestActorService2MessageHandler::request_msg2_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const request_msg2* pRequest)
{
	response_msg2 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);
}