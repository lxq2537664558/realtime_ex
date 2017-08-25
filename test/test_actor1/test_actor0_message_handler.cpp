#include "test_actor0_message_handler.h"

#include "libCoreCommon/core_common.h"
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"

CTestActor0MessageHandler::CTestActor0MessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_ACTOR_MESSAGE_HANDLER(pServiceBase, CTestActor0MessageHandler, "request_msg0", &CTestActor0MessageHandler::request_msg0_handler);
}

CTestActor0MessageHandler::~CTestActor0MessageHandler()
{

}

bool CTestActor0MessageHandler::request_msg0_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg0* pRequest)
{
	response_msg0 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	pActorBase->getServiceBase()->getServiceInvoker()->response(sSessionInfo, &msg1);

	return true;
}