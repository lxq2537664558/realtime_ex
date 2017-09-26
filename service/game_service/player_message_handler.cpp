#include "player_message_handler.h"
#include "player.h"
#include "game_service.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/register_message_util.h"

#include "client_proto_src/c2s_update_name_response.pb.h"
#include "client_proto_src/c2s_gm_command_response.pb.h"

#include "../common/error_code.h"

CPlayerMessageHandler::CPlayerMessageHandler(CServiceBase* pServiceBase)
{
	register_pb_actor_forward_handler(pServiceBase, this, &CPlayerMessageHandler::c2s_update_name_request_handler);
	register_pb_actor_forward_handler(pServiceBase, this, &CPlayerMessageHandler::c2s_gm_command_request_handler);
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

	pPlayer->sendClientMessage(&response_msg);
}

void CPlayerMessageHandler::c2s_gm_command_request_handler(CActorBase* pActorBase, SClientSessionInfo sClientSessionInfo, const c2s_gm_command_request* pRequest)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pActorBase);
	DebugAst(pPlayer != nullptr);

	CGameService* pGameService = dynamic_cast<CGameService*>(pPlayer->getServiceBase());
	DebugAst(pGameService != nullptr);

	uint32_t nRet = pGameService->getGMCommandMgr()->dispatchCommand(pPlayer, pRequest->name(), pRequest->arg());
	
	PrintInfo("gm command player_id: {} name: {} arg: {}", pPlayer->getActorID(), pRequest->name(), pRequest->arg());
	
	c2s_gm_command_response response_msg;
	response_msg.set_name(pRequest->name());
	response_msg.set_result(nRet);

	pPlayer->sendClientMessage(&response_msg);
}