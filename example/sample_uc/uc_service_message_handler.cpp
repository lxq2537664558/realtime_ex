#include "uc_service_message_handler.h"
#include "uc_service.h"

#include "libCoreCommon/actor_base.h"
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/service_invoker.h"

#include "msg_proto_src/d2u_player_login_response.pb.h"
#include "msg_proto_src/u2s_kick_player_notify.pb.h"

CUCServiceMessageHandler::CUCServiceMessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CUCServiceMessageHandler, "d2u_player_login_request", &CUCServiceMessageHandler::d2u_player_login_request_handler);
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CUCServiceMessageHandler, "s2u_player_heartbeat_notify", &CUCServiceMessageHandler::s2u_player_heartbeat_notify_handler);
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CUCServiceMessageHandler, "s2u_player_leave_notify", &CUCServiceMessageHandler::s2u_player_leave_notify_handler);
}

CUCServiceMessageHandler::~CUCServiceMessageHandler()
{

}

void CUCServiceMessageHandler::d2u_player_login_request_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const d2u_player_login_request* pRequest)
{
	CUCService* pUCService = dynamic_cast<CUCService*>(pServiceBase);
	DebugAst(pUCService != nullptr);

	CUserInfo* pUserInfo = pUCService->getUserInfoMgr()->getUserInfo(pRequest->player_id());
	if (pUserInfo == nullptr)
	{
		pUserInfo = pUCService->getUserInfoMgr()->createUserInfo(pRequest->player_id(), pRequest->gas_id());
		DebugAst(pUserInfo != nullptr);

		PrintInfo("CUCServiceMessageHandler::d2u_player_login_request_handler 1 player_id: {} gas_id: {}", pRequest->player_id(), pRequest->gas_id());

		pUserInfo->startTimout();
	}
	else
	{
		PrintInfo("CUCServiceMessageHandler::d2u_player_login_request_handler 2 player_id: {} old_gas_id: {} new_gas_id: {}", pRequest->player_id(), pUserInfo->getGasID(), pRequest->gas_id());
		
		if (pUserInfo->getGasID() == 0)
		{
			pUserInfo->setGasID(pRequest->gas_id());
			pUserInfo->startTimout();
		}
	}

	d2u_player_login_response response_msg;
	response_msg.set_player_id(pUserInfo->getPlayerID());
	response_msg.set_gas_id(pUserInfo->getGasID());

	pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
}

void CUCServiceMessageHandler::s2u_player_heartbeat_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2u_player_heartbeat_notify* pRequest)
{
	CUCService* pUCService = dynamic_cast<CUCService*>(pServiceBase);
	DebugAst(pUCService != nullptr);

	CUserInfo* pUserInfo = pUCService->getUserInfoMgr()->getUserInfo(pRequest->player_id());
	if (nullptr == pUserInfo)
	{
		u2s_kick_player_notify notify_msg;
		notify_msg.set_player_id(pRequest->player_id());

		pServiceBase->getServiceInvoker()->send(eMTT_Service, sSessionInfo.nFromServiceID, &notify_msg);
		return;
	}

	if (pUserInfo->getGasID() != sSessionInfo.nFromServiceID)
	{
		pUserInfo->setGasID(0);

		u2s_kick_player_notify notify_msg;
		notify_msg.set_player_id(pRequest->player_id());

		pServiceBase->getServiceInvoker()->send(eMTT_Service, sSessionInfo.nFromServiceID, &notify_msg);
		return;
	}

	pUserInfo->onHeartbeat();
}

void CUCServiceMessageHandler::s2u_player_leave_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2u_player_leave_notify* pRequest)
{
	CUCService* pUCService = dynamic_cast<CUCService*>(pServiceBase);
	DebugAst(pUCService != nullptr);

	CUserInfo* pUserInfo = pUCService->getUserInfoMgr()->getUserInfo(pRequest->player_id());
	if (nullptr == pUserInfo)
		return;

	PrintInfo("CUCServiceMessageHandler::s2u_player_leave_notify_handler player_id: {}", pRequest->player_id());

	pUserInfo->setGasID(0);
}