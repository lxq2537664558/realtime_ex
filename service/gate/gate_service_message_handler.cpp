#include "gate_service_message_handler.h"
#include "gate_service.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"
#include "libCoreCommon/register_message_util.h"

using namespace core;

CGateServiceMessageHandler::CGateServiceMessageHandler(CServiceBase* pServiceBase)
{
	register_pb_service_message_handler(pServiceBase, this, &CGateServiceMessageHandler::d2g_player_token_notify_handler);
	register_pb_service_message_handler(pServiceBase, this, &CGateServiceMessageHandler::s2g_kick_player_notify_handler);
}

CGateServiceMessageHandler::~CGateServiceMessageHandler()
{

}

void CGateServiceMessageHandler::d2g_player_token_notify_handler(core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const d2g_player_token_notify* pRequest)
{
	CGateService* pGateService = dynamic_cast<CGateService*>(pServiceBase);
	DebugAst(pGateService != nullptr);

	CGateClientSession* pGateClientSession = pGateService->getGateClientSessionMgr()->getSessionByPlayerID(pRequest->player_id());
	if (pGateClientSession != nullptr)
	{
		// 匹配
		if ((pGateClientSession->getState()&eCSS_ClientEnter) != 0)
		{
			// token不一致，直接踢掉
			if (pGateClientSession->getToken() != pRequest->token())
			{
				PrintWarning("CGateServiceMessageHandler::d2g_player_token_notify_handler token error player_id: {}", pRequest->player_id());

				CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pGateClientSession->getSocketID());
				if (nullptr == pBaseConnection)
				{
					PrintWarning("CGateServiceMessageHandler::d2g_player_token_notify_handler nullptr == pBaseConnection player_id: {}", pGateClientSession->getPlayerID());
					pGateService->getGateClientSessionMgr()->destroySession(pGateClientSession->getPlayerID(), "error");
					return;
				}
				pBaseConnection->shutdown(true, "token error");
				return;
			}

			PrintInfo("CGateServiceMessageHandler::d2g_player_token_notify_handler 1 player_id: {} token: {} gas_id: {}", pGateClientSession->getPlayerID(), pGateClientSession->getToken(), pRequest->gas_id());

			pGateClientSession->setGasID(pRequest->gas_id());
			pGateClientSession->setState(eCSS_TokenEnter|eCSS_ClientEnter);
			pGateClientSession->enterGas();

			return;
		}

		// 重复的dispatch发来的token，直接踢掉
		if (pGateClientSession->getState() != eCSS_Normal)
		{
			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pGateClientSession->getSocketID());
			if (nullptr == pBaseConnection)
			{
				PrintWarning("CGateServiceMessageHandler::d2g_player_token_notify_handler nullptr == pBaseConnection player_id: {}", pGateClientSession->getPlayerID());
				pGateService->getGateClientSessionMgr()->destroySession(pGateClientSession->getPlayerID(), "error");
				return;
			}

			PrintInfo("CGateServiceMessageHandler::d2g_player_token_notify_handler 2 player_id: {} token: {} old_gas_id: {} new_gas_id: {}", pGateClientSession->getPlayerID(), pGateClientSession->getToken(), pGateClientSession->getGasID(), pRequest->gas_id());

			pBaseConnection->shutdown(true, "dup");
			return;
		}

		PrintInfo("CGateServiceMessageHandler::d2g_player_token_notify_handler 3 player_id: {} token: {} old_gas_id: {} new_gas_id: {}", pGateClientSession->getPlayerID(), pGateClientSession->getToken(), pGateClientSession->getGasID(), pRequest->gas_id());
		
		// 顶号
		uint64_t nOldSocketID = pGateClientSession->getSocketID();
		pGateService->getGateClientSessionMgr()->unbindSocketID(pGateClientSession->getPlayerID());

		// 把老的链接踢掉
		CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(nOldSocketID);
		if (nullptr != pBaseConnection)
			pBaseConnection->shutdown(true, "dup");

		pGateClientSession->setToken(pRequest->token());
		pGateClientSession->setState(eCSS_TokenEnter);
	}
	else
	{
		PrintInfo("CGateServiceMessageHandler::d2g_player_token_notify_handler 4 player_id: {} token: {} gas_id: {}", pRequest->player_id(), pRequest->token(), pRequest->gas_id());

		// 第一次来
		pGateClientSession = pGateService->getGateClientSessionMgr()->createSession(pRequest->player_id(), pRequest->token());
		if (nullptr == pGateClientSession)
		{
			PrintWarning("CGateServiceMessageHandler::d2g_player_token_notify_handler error create session error player_id: {}", pRequest->player_id());
			return;
		}

		pGateClientSession->setGasID(pRequest->gas_id());
		pGateClientSession->setState(eCSS_TokenEnter);
	}
}

void CGateServiceMessageHandler::s2g_kick_player_notify_handler(core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2g_kick_player_notify* pRequest)
{
	CGateService* pGateService = dynamic_cast<CGateService*>(pServiceBase);
	DebugAst(pGateService != nullptr);

	CGateClientSession* pGateClientSession = pGateService->getGateClientSessionMgr()->getSessionByPlayerID(pRequest->player_id());
	if (nullptr == pGateClientSession)
		return;

	pGateClientSession->setKick(true);
	pGateService->getGateClientSessionMgr()->destroySession(pRequest->player_id(), "gas kick");
}