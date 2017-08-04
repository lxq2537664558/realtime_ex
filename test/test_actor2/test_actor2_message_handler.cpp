#include "test_actor2_message_handler.h"

CTestActor2MessageHandler::CTestActor2MessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_ACTOR_MESSAGE_HANDLER(pServiceBase, CTestActor2MessageHandler, "request_msg3", &CTestActor2MessageHandler::request_msg3_handler);
}

CTestActor2MessageHandler::~CTestActor2MessageHandler()
{

}

bool CTestActor2MessageHandler::request_msg3_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg3* pRequest)
{
	response_msg3 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	pActorBase->response(sSessionInfo, &msg1);

	return true;
}