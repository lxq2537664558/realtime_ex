#include "test_actor2_message_handler.h"

CTestActor2MessageHandler::CTestActor2MessageHandler(CServiceBase* pServiceBase)
	: m_pServiceBase(pServiceBase)
{
}

CTestActor2MessageHandler::~CTestActor2MessageHandler()
{

}

bool CTestActor2MessageHandler::init()
{
	REGISTER_ACTOR_MESSAGE_HANDLER(this->m_pServiceBase, CTestActor2MessageHandler, "request_msg3", &CTestActor2MessageHandler::request_msg3_handler);

	return true;
}

bool CTestActor2MessageHandler::request_msg3_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg3* pRequest)
{
	response_msg3 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	this->m_pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);

	return true;
}