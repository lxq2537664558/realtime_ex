#include "player_message_handler.h"
#include "player.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"

#include "msg_proto_src/c2s_update_name_response.pb.h"

CPlayerMessageHandler::CPlayerMessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_ACTOR_FORWARD_HANDLER(pServiceBase, CPlayerMessageHandler, "c2s_update_name_request", &CPlayerMessageHandler::c2s_update_name_request_handler);
}

CPlayerMessageHandler::~CPlayerMessageHandler()
{

}

void CPlayerMessageHandler::c2s_update_name_request_handler(CActorBase* pActorBase, SClientSessionInfo sClientSessionInfo, const c2s_update_name_request* pRequest)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pActorBase);
	DebugAst(pPlayer != nullptr);

	c2s_update_name_response response_msg;
	response_msg.set_name(pRequest->name());

	pActorBase->getServiceBase()->getServiceInvoker()->send(sClientSessionInfo, &response_msg);
}