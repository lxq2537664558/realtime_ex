#include "gas_service_message_handler.h"
#include "gas_service.h"

#include "libCoreCommon/actor_base.h"
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"
#include "libBaseCommon/string_util.h"

#include "../common/common.h"

#include "msg_proto_src/g2s_player_enter_response.pb.h"
#include "msg_proto_src/s2g_kick_player_notify.pb.h"
#include "msg_proto_src/s2u_player_leave_notify.pb.h"

CGasServiceMessageHandler::CGasServiceMessageHandler(CServiceBase* pServiceBase)
{
	register_service_message_handler(pServiceBase, this, &CGasServiceMessageHandler::g2s_player_enter_request_handler);
	register_service_message_handler(pServiceBase, this, &CGasServiceMessageHandler::g2s_player_leave_notify_handler);
	register_service_message_handler(pServiceBase, this, &CGasServiceMessageHandler::u2s_kick_player_notify_handler);
	register_service_message_handler(pServiceBase, this, &CGasServiceMessageHandler::g2s_player_heartbeat_notify_handler);
}

CGasServiceMessageHandler::~CGasServiceMessageHandler()
{

}

void CGasServiceMessageHandler::g2s_player_enter_request_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_enter_request* pRequest)
{
	CGasService* pGasService = dynamic_cast<CGasService*>(pServiceBase);
	DebugAst(pGasService != nullptr);

	g2s_player_enter_response response_msg;
	response_msg.set_player_id(pRequest->player_id());

	CPlayer* pPlayer = pGasService->getPlayerMgr()->getPlayer(pRequest->player_id());
	if (pPlayer == nullptr)
	{
		std::string szContext = base::string_util::convert_to_str(sSessionInfo.nFromServiceID);

		pPlayer = pGasService->getPlayerMgr()->createPlayer(pRequest->player_id(), szContext);
		
		if (nullptr == pPlayer)
		{
			response_msg.set_result(1);
			pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
			return;
		}

		response_msg.set_result(0);
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);

		pPlayer->onPlayerLogin();
	}
	else
	{
		// 先退出
		pPlayer->onPlayerLogout();

		pPlayer->setGateServiceID(sSessionInfo.nFromServiceID);

		response_msg.set_result(0);
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
		
		// 再重新登录
		pPlayer->onPlayerLogin();
	}
}

void CGasServiceMessageHandler::g2s_player_leave_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_leave_notify* pMessage)
{
	CGasService* pGasService = dynamic_cast<CGasService*>(pServiceBase);
	DebugAst(pGasService != nullptr);

	CPlayer* pPlayer = pGasService->getPlayerMgr()->getPlayer(pMessage->player_id());
	if (nullptr == pPlayer)
		return;

	pPlayer->onPlayerLogout();
	pGasService->getPlayerMgr()->destroyPlayer(pMessage->player_id(), "gate notify leave");

	uint32_t nDBID = _GET_PLAYER_DB_ID(pMessage->player_id());
	uint32_t nUCServiceID = nDBID + _UC_SERVICE_DELTA;
	s2u_player_leave_notify notify_msg;
	notify_msg.set_player_id(pMessage->player_id());
	pServiceBase->getServiceInvoker()->send(nUCServiceID, &notify_msg);
}

void CGasServiceMessageHandler::u2s_kick_player_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const u2s_kick_player_notify* pMessage)
{
	CGasService* pGasService = dynamic_cast<CGasService*>(pServiceBase);
	DebugAst(pGasService != nullptr);

	CPlayer* pPlayer = pGasService->getPlayerMgr()->getPlayer(pMessage->player_id());
	if (nullptr == pPlayer)
		return;

	pPlayer->onPlayerLogout();
	uint32_t nGateID = pPlayer->getGateServiceID();
	pGasService->getPlayerMgr()->destroyPlayer(pMessage->player_id(), "uc kick");

	s2g_kick_player_notify notify_msg;
	notify_msg.set_player_id(pMessage->player_id());
	pGasService->getServiceInvoker()->send(nGateID, &notify_msg);
}

void CGasServiceMessageHandler::g2s_player_heartbeat_notify_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const g2s_player_heartbeat_notify* pMessage)
{
	CGasService* pGasService = dynamic_cast<CGasService*>(pServiceBase);
	DebugAst(pGasService != nullptr);

	CPlayer* pPlayer = pGasService->getPlayerMgr()->getPlayer(pMessage->player_id());
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