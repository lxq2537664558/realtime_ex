#include "test_actor0_message_handler.h"

CTestActor0MessageHandler::CTestActor0MessageHandler(CServiceBase* pServiceBase)
	: m_pServiceBase(pServiceBase)
{
}

CTestActor0MessageHandler::~CTestActor0MessageHandler()
{

}

bool CTestActor0MessageHandler::init()
{
	REGISTER_ACTOR_MESSAGE_HANDLER(this->m_pServiceBase, CTestActor0MessageHandler, "request_msg0", &CTestActor0MessageHandler::request_msg0_handler);

	return true;
}

bool CTestActor0MessageHandler::request_msg0_handler(CActorBase* pActorBase, SSessionInfo sSessionInfo, const request_msg0* pRequest)
{
	response_msg0 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	this->m_pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);

	return true;
}