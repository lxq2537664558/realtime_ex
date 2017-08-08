#include "dispatch_service_message_handler.h"
#include "dispatch_service.h"
#include "libBaseCommon/rand_gen.h"
#include "libCoreCommon/service_invoker.h"

#include "proto_src/validate_login_response.pb.h"
#include "proto_src/player_token_notify.pb.h"

CDispatchServiceMessageHandler::CDispatchServiceMessageHandler(CServiceBase* pServiceBase)
{
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CDispatchServiceMessageHandler, "gas_online_count_notify", &CDispatchServiceMessageHandler::gas_online_count_handler);
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CDispatchServiceMessageHandler, "gate_online_count_notify", &CDispatchServiceMessageHandler::gate_online_count_handler);
	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CDispatchServiceMessageHandler, "validate_login_request", &CDispatchServiceMessageHandler::validate_login_handler);

	REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, CDispatchServiceMessageHandler, "gate_addr_notify", &CDispatchServiceMessageHandler::gate_addr_handler);
}

CDispatchServiceMessageHandler::~CDispatchServiceMessageHandler()
{

}

void CDispatchServiceMessageHandler::gas_online_count_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const gas_online_count_notify* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	pDispatchService->getOnlineCountMgr()->setGasOnlineCount(sSessionInfo.nFromServiceID, pRequest->count());
}

void CDispatchServiceMessageHandler::gate_online_count_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const gate_online_count_notify* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	pDispatchService->getOnlineCountMgr()->setGateOnlineCount(sSessionInfo.nFromServiceID, pRequest->count());
}

void CDispatchServiceMessageHandler::validate_login_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const validate_login_request* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	PrintInfo("CDispatchServiceMessageHandler::validate_login_handler account_id: "UINT64FMT" server_id: %d", pRequest->account_id(), pRequest->server_id());

	uint64_t nPlayerID = (uint64_t)pRequest->server_id() << 32 | pRequest->account_id();
	char szToken[256] = { 0 };
	base::crt::snprintf(szToken, _countof(szToken), "%d", base::CRandGen::getGlobalRand(1000000));

	uint32_t nGasID = pDispatchService->getOnlineCountMgr()->getSuitableGasID();
	uint32_t nGateID = pDispatchService->getOnlineCountMgr()->getSuitableGateID();
	
	player_token_notify msg;
	msg.set_player_id(nPlayerID);
	msg.set_gas_id(nGasID);
	msg.set_token(szToken);

	pServiceBase->getServiceInvoker()->send(eMTT_Service, nGateID, &msg);

	char szKey[256] = { 0 };
	base::crt::snprintf(szKey, _countof(szKey), UINT64FMT"|%s", nPlayerID, szToken);
	validate_login_response response_msg;
	response_msg.set_key(szKey);
	response_msg.set_result(0);
	response_msg.set_gate_addr(pDispatchService->getOnlineCountMgr()->getGateAddr(nGateID));

	pServiceBase->getServiceInvoker()->response(sSessionInfo, &response_msg);
}

void CDispatchServiceMessageHandler::gate_addr_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const gate_addr_notify* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	pDispatchService->getOnlineCountMgr()->setGateAddr(sSessionInfo.nFromServiceID, pRequest->addr());
}
