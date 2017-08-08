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

	CClientSession* pClientSession = pGateService->getClientSessionMgr()->getSessionByPlayerID(pRequest->player_id());
	if (pClientSession != nullptr)
	{
		// 顶号
		if ((pClientSession->getState()&eCSS_TokenEnter) != 0)
		{
			// 这里先直接踢掉
			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pClientSession->getSocketID());
			if (nullptr == pBaseConnection)
			{
				PrintWarning("nullptr == pBaseConnection player_id: "UINT64FMT, pClientSession->getPlayerID());
				pGateService->getClientSessionMgr()->delSessionByPlayerID(pClientSession->getPlayerID());
				return;
			}
			pBaseConnection->shutdown(true, "dup");
			return;
		}
		DebugAst(pClientSession->getState() == eCSS_ClientEnter);

		if (pClientSession->getToken() != pRequest->token())
		{
			PrintWarning("CGateServiceMessageHandler::player_token_handler token error player_id: "UINT64FMT, pRequest->player_id());

			CBaseConnection* pBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnectionBySocketID(pClientSession->getSocketID());
			if (nullptr == pBaseConnection)
			{
				PrintWarning("nullptr == pBaseConnection player_id: "UINT64FMT, pClientSession->getPlayerID());
				pGateService->getClientSessionMgr()->delSessionByPlayerID(pClientSession->getPlayerID());
				return;
			}
			pBaseConnection->shutdown(true, "token error");
			return;
		}

		pClientSession->setServiceID(pRequest->gas_id());
		pClientSession->setState(eCSS_TokenEnter);
		pClientSession->enterGas(pGateService);
	}
	else
	{
		pClientSession = pGateService->getClientSessionMgr()->createSession(pRequest->player_id(), pRequest->token());
		if (nullptr == pClientSession)
		{
			PrintWarning("CGateServiceMessageHandler::player_token_handler error create session error player_id: "UINT64FMT, pRequest->player_id());
			return;
		}

		pClientSession->setServiceID(pRequest->gas_id());
		pClientSession->setState(eCSS_TokenEnter);
	}
}
