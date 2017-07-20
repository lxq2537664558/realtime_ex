#include "test_service3_message_handler.h"
#include "../proto_src/request_msg2.pb.h"
#include "../proto_src/response_msg2.pb.h"


CTestService3MessageHandler::CTestService3MessageHandler(CServiceBase* pServiceBase)
	: m_pServiceBase(pServiceBase)
{
}

CTestService3MessageHandler::~CTestService3MessageHandler()
{

}

bool CTestService3MessageHandler::init()
{
	REGISTER_SERVICE_MESSAGE_HANDLER(this->m_pServiceBase, CTestService3MessageHandler, "request_msg2", &CTestService3MessageHandler::request_msg2_handler);

	return true;
}

bool CTestService3MessageHandler::request_msg2_handler(SSessionInfo sSessionInfo, const request_msg2* pRequest)
{
	response_msg2 msg1;
	msg1.set_id(pRequest->id());
	msg1.set_name(pRequest->name());

	this->m_pServiceBase->getServiceInvoker()->response(sSessionInfo, &msg1);

	return true;
}