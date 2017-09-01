#include "test_actor0_message_handler.h"

#include "libCoreCommon/core_common.h"
#include "libCoreCommon/service_invoker.h"

CTestActor0MessageHandler::CTestActor0MessageHandler(CServiceBase* pServiceBase)
{
	register_actor_message_handler(pServiceBase, this, &CTestActor0MessageHandler::request_msg0_handler);
}

CTestActor0MessageHandler::~CTestActor0MessageHandler()
{

}

void CTestActor0MessageHandler::request_msg0_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg0* pRequest)
{
	response_msg0 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	pActorBase->getServiceBase()->getServiceInvoker()->response(sSessionInfo, &msg1);
}