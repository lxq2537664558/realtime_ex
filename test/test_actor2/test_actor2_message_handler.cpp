#include "test_actor2_message_handler.h"
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"

CTestActor2MessageHandler::CTestActor2MessageHandler(CServiceBase* pServiceBase)
{
	register_actor_message_handler(pServiceBase, this, &CTestActor2MessageHandler::request_msg3_handler);
}

CTestActor2MessageHandler::~CTestActor2MessageHandler()
{

}

void CTestActor2MessageHandler::request_msg3_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg3* pRequest)
{
	response_msg3 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	pActorBase->getServiceBase()->getServiceInvoker()->response(sSessionInfo, &msg1);
}