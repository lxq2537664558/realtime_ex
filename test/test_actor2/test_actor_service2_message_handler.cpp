#include "test_actor_service2_message_handler.h"

CTestActorService2MessageHandler::CTestActorService2MessageHandler(CServiceBase* pServiceBase)
	: m_pServiceBase(pServiceBase)
{
}

CTestActorService2MessageHandler::~CTestActorService2MessageHandler()
{

}

bool CTestActorService2MessageHandler::init()
{
	REGISTER_SERVICE_MESSAGE_HANDLER(this->m_pServiceBase, CTestActorService2MessageHandler, "request_msg2", &CTestActorService2MessageHandler::request_msg2_handler);

	return true;
}

bool CTestActorService2MessageHandler::request_msg2_handler(SSessionInfo sSessionInfo, const request_msg2* pRequest)
{
	response_msg2 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	this->m_pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);

	return true;
}