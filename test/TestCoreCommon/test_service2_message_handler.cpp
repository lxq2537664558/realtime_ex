#include "test_service2_message_handler.h"
#include "response_msg1.pb.h"


CTestService2MessageHandler::CTestService2MessageHandler(CServiceBase* pServiceBase)
	: m_pServiceBase(pServiceBase)
{

}

CTestService2MessageHandler::~CTestService2MessageHandler()
{

}

bool CTestService2MessageHandler::init()
{
	REGISTER_SERVICE_MESSAGE_HANDLER(this->m_pServiceBase, CTestService2MessageHandler, "request_msg1", &CTestService2MessageHandler::request_msg1_handler);

	return true;
}

bool CTestService2MessageHandler::request_msg1_handler(SSessionInfo sSessionInfo, const request_msg1* pRequest)
{
	response_msg1 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	this->m_pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);

	return true;
}