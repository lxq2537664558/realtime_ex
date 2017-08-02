#include "watchdog_service_message_handler.h"

#include "libCoreCommon\actor_base.h"

#include "..\..\service\gate\proto_src\player_enter_response.pb.h"


CWatchdogServiceMessageHandler::CWatchdogServiceMessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CWatchdogServiceMessageHandler, "player_enter_request", &CWatchdogServiceMessageHandler::player_enter_handler);
}

CWatchdogServiceMessageHandler::~CWatchdogServiceMessageHandler()
{

}

void CWatchdogServiceMessageHandler::player_enter_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const player_enter_request* pRequest)
{
	player_enter_response response_msg;
	response_msg.set_player_id(pRequest->player_id());

	CActorBase* pActorBase = pServiceBase->createActor("CPlayer", pRequest->player_id(), "");
	if (nullptr == pActorBase)
	{
		response_msg.set_result(1);
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
		return;
	}

	response_msg.set_result(0);
	pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
}

void CWatchdogServiceMessageHandler::player_leave_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const player_leave* pMessage)
{
	CActorBase* pActorBase = pServiceBase->getActorBase(pMessage->player_id());
	if (nullptr == pActorBase)
		return;

	pActorBase->release();
}