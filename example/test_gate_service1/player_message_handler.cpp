#include "player_message_handler.h"
#include "player.h"
#include "proto_src\update_name_response.pb.h"

CPlayerMessageHandler::CPlayerMessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_ACTOR_FORWARD_HANDLER(pServiceBase, CPlayerMessageHandler, "update_name_request", &CPlayerMessageHandler::update_name_handler);
}

CPlayerMessageHandler::~CPlayerMessageHandler()
{

}

void CPlayerMessageHandler::update_name_handler(CActorBase* pActorBase, SClientSessionInfo sClientSessionInfo, const update_name_request* pRequest)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(pActorBase);
	DebugAst(pPlayer != nullptr);

	pPlayer->setName(pRequest->name());

	update_name_response response_msg;
	response_msg.set_name(pPlayer->getName());

	pActorBase->send(sClientSessionInfo, &response_msg);
}