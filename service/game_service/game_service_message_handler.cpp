#include "game_service_message_handler.h"
#include "game_service.h"

#include "libBaseCommon/string_util.h"

#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/register_message_util.h"

#include "../common/common.h"
#include "../common/error_code.h"

#include "server_proto_src/g2s_player_enter_response.pb.h"
#include "server_proto_src/s2g_kick_player_notify.pb.h"
#include "server_proto_src/s2u_player_leave_notify.pb.h"

CGameServiceMessageHandler::CGameServiceMessageHandler(CServiceBase* pServiceBase)
{
	register_service_message_handler(pServiceBase, this, &CGameServiceMessageHandler::g2s_player_enter_request_handler);
	register_service_message_handler(pServiceBase, this, &CGameServiceMessageHandler::g2s_player_leave_notify_handler);
	register_service_message_handler(pServiceBase, this, &CGameServiceMessageHandler::u2s_kick_player_notify_handler);
	register_service_message_handler(pServiceBase, this, &CGameServiceMessageHandler::g2s_player_heartbeat_notify_handler);
}

CGameServiceMessageHandler::~CGameServiceMessageHandler()
{

}

void CGameServiceMessageHandler::g2s_player_enter_request_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_enter_request* pRequest)
{
	CGameService* pGameService = dynamic_cast<CGameService*>(pServiceBase);
	DebugAst(pGameService != nullptr);

	g2s_player_enter_response response_msg;
	response_msg.set_player_id(pRequest->player_id());

	CPlayer* pPlayer = pGameService->getPlayerMgr()->getPlayer(pRequest->player_id());
	if (pPlayer == nullptr)
	{
		SCreatePlayerContext sCreatePlayerContext;
		sCreatePlayerContext.nGateServiceID = sSessionInfo.nFromServiceID;
		sCreatePlayerContext.nPlayerID = pRequest->player_id();
		pPlayer = pGameService->getPlayerMgr()->createPlayer(pRequest->player_id(), &sCreatePlayerContext);
		
		if (nullptr == pPlayer)
		{
			response_msg.set_result(eEC_Login_CreatePlayerError);
			pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
			return;
		}

		response_msg.set_result(eEC_Sucess);
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);

		pPlayer->onPlayerLogin();
	}
	else
	{
		uint32_t nGateID = pPlayer->getGateServiceID();
		// 先退出
		pPlayer->onPlayerLogout();
		
		// 来自不一样的gate，踢掉旧的，如果来自同一个gate，gate那边能自己处理掉
		if (nGateID != sSessionInfo.nFromServiceID)
		{
			s2g_kick_player_notify notify_msg;
			notify_msg.set_player_id(pPlayer->getPlayerID());
			pGameService->getServiceInvoker()->send(nGateID, &notify_msg);
		}

		pPlayer->setGateServiceID(sSessionInfo.nFromServiceID);

		response_msg.set_result(eEC_Sucess);
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
		
		// 再重新登录
		pPlayer->onPlayerLogin();
	}
}

void CGameServiceMessageHandler::g2s_player_leave_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_leave_notify* pMessage)
{
	CGameService* pGameService = dynamic_cast<CGameService*>(pServiceBase);
	DebugAst(pGameService != nullptr);

	CPlayer* pPlayer = pGameService->getPlayerMgr()->getPlayer(pMessage->player_id());
	if (nullptr != pPlayer)
	{
		uint32_t nUCServiceID = pPlayer->getUCServiceID();

		pPlayer->onPlayerLogout();

		s2u_player_leave_notify notify_msg;
		notify_msg.set_player_id(pMessage->player_id());
		pServiceBase->getServiceInvoker()->send(nUCServiceID, &notify_msg);
	}

	pGameService->getPlayerMgr()->destroyPlayer(pMessage->player_id(), "gate notify leave");
}

void CGameServiceMessageHandler::u2s_kick_player_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const u2s_kick_player_notify* pMessage)
{
	CGameService* pGameService = dynamic_cast<CGameService*>(pServiceBase);
	DebugAst(pGameService != nullptr);

	CPlayer* pPlayer = pGameService->getPlayerMgr()->getPlayer(pMessage->player_id());
	if (nullptr != pPlayer)
	{
		pPlayer->onPlayerLogout();

		uint32_t nGateID = pPlayer->getGateServiceID();
		
		s2g_kick_player_notify notify_msg;
		notify_msg.set_player_id(pMessage->player_id());
		pGameService->getServiceInvoker()->send(nGateID, &notify_msg);
	}

	pGameService->getPlayerMgr()->destroyPlayer(pMessage->player_id(), "uc kick");
}

void CGameServiceMessageHandler::g2s_player_heartbeat_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_heartbeat_notify* pMessage)
{
	CGameService* pGameService = dynamic_cast<CGameService*>(pServiceBase);
	DebugAst(pGameService != nullptr);

	CPlayer* pPlayer = pGameService->getPlayerMgr()->getPlayer(pMessage->player_id());
	if (nullptr == pPlayer)
	{
		s2g_kick_player_notify notify_msg;
		notify_msg.set_player_id(pMessage->player_id());

		pServiceBase->getServiceInvoker()->send(sSessionInfo.nFromServiceID, &notify_msg);
		return;
	}

	if (pPlayer->getGateServiceID() != sSessionInfo.nFromServiceID)
	{
		s2g_kick_player_notify notify_msg;
		notify_msg.set_player_id(pMessage->player_id());

		pServiceBase->getServiceInvoker()->send(sSessionInfo.nFromServiceID, &notify_msg);
		return;
	}
}