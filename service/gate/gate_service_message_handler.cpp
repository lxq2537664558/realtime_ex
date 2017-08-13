#include "stdafx.h"
#include "gate_service_message_handler.h"
#include "gate_service.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

using namespace core;

CGateServiceMessageHandler::CGateServiceMessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CGateServiceMessageHandler, "player_token_notify", &CGateServiceMessageHandler::player_token_handler);
}

CGateServiceMessageHandler::~CGateServiceMessageHandler()
{

}

void CGateServiceMessageHandler::player_token_handler(core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const player_token_notify* pRequest)
{
	CGateService* pGateService = dynamic_cast<CGateService*>(pServiceBase);
	DebugAst(pGateService != nullptr);

	CGateClientSession* pGateClientSession = pGateService->getGateClientSessionMgr()->getSessionByPlayerID(pRequest->player_id());
	if (pGateClientSession != nullptr)
	{
		// 顶号
		if ((pGateClientSession->getState()&eCSS_TokenEnter) != 0)
		{
			// 这里先直接踢掉
			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pGateClientSession->getSocketID());
			if (nullptr == pBaseConnection)
			{
				PrintWarning("nullptr == pBaseConnection player_id: "UINT64FMT, pGateClientSession->getPlayerID());
				pGateService->getGateClientSessionMgr()->delSessionByPlayerID(pGateClientSession->getPlayerID());
				return;
			}
			pBaseConnection->shutdown(true, "dup");
			return;
		}
		DebugAst(pGateClientSession->getState() == eCSS_ClientEnter);

		if (pGateClientSession->getToken() != pRequest->token())
		{
			PrintWarning("CGateServiceMessageHandler::player_token_handler token error player_id: "UINT64FMT, pRequest->player_id());

			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pGateClientSession->getSocketID());
			if (nullptr == pBaseConnection)
			{
				PrintWarning("nullptr == pBaseConnection player_id: "UINT64FMT, pGateClientSession->getPlayerID());
				pGateService->getGateClientSessionMgr()->delSessionByPlayerID(pGateClientSession->getPlayerID());
				return;
			}
			pBaseConnection->shutdown(true, "token error");
			return;
		}

		pGateClientSession->setServiceID(pRequest->gas_id());
		pGateClientSession->setState(eCSS_TokenEnter);
		pGateClientSession->enterGas();
	}
	else
	{
		pGateClientSession = pGateService->getGateClientSessionMgr()->createSession(pRequest->player_id(), pRequest->token());
		if (nullptr == pGateClientSession)
		{
			PrintWarning("CGateServiceMessageHandler::player_token_handler error create session error player_id: "UINT64FMT, pRequest->player_id());
			return;
		}

		pGateClientSession->setServiceID(pRequest->gas_id());
		pGateClientSession->setState(eCSS_TokenEnter);
	}
}
