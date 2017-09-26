#include "dispatch_service_message_handler.h"
#include "dispatch_service.h"

#include "libBaseCommon/rand_gen.h"
#include "libBaseCommon/string_util.h"
#include "libBaseCommon/time_util.h"

#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/register_message_util.h"

#include "db_proto_src/result_set.pb.h"
#include "server_proto_src/l2d_validate_login_response.pb.h"
#include "server_proto_src/d2g_player_token_notify.pb.h"
#include "server_proto_src/d2u_player_login_request.pb.h"
#include "server_proto_src/d2u_player_login_response.pb.h"
#include "../common/common.h"
#include "../common/error_code.h"

#define _ACCOUNT_TBL_PREFIX "account_base_"

CDispatchServiceMessageHandler::CDispatchServiceMessageHandler(CDispatchService* pDispatchService)
	: CDbServiceInvokeHolder(pDispatchService, pDispatchService->getGlobalDbServiceID())
	, CServiceInvokeHolder(pDispatchService)
{
	register_pb_service_message_handler(pDispatchService, this, &CDispatchServiceMessageHandler::s2d_online_count_notify_handler);
	register_pb_service_message_handler(pDispatchService, this, &CDispatchServiceMessageHandler::g2d_online_count_notify_handler);
	register_pb_service_message_handler(pDispatchService, this, &CDispatchServiceMessageHandler::l2d_validate_login_request_handler);
	register_pb_service_message_handler(pDispatchService, this, &CDispatchServiceMessageHandler::g2d_addr_notify_handler);
	register_pb_service_message_handler(pDispatchService, this, &CDispatchServiceMessageHandler::u2d_active_count_notify_handler);
}

CDispatchServiceMessageHandler::~CDispatchServiceMessageHandler()
{

}

void CDispatchServiceMessageHandler::s2d_online_count_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2d_online_count_notify* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	pDispatchService->getOnlineCountMgr()->setGasOnlineCount(sSessionInfo.nFromServiceID, pRequest->count());
}

void CDispatchServiceMessageHandler::g2d_online_count_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const g2d_online_count_notify* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	pDispatchService->getOnlineCountMgr()->setGateOnlineCount(sSessionInfo.nFromServiceID, pRequest->count());
}

void CDispatchServiceMessageHandler::g2d_addr_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const g2d_addr_notify* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	pDispatchService->getOnlineCountMgr()->setGateAddr(sSessionInfo.nFromServiceID, pRequest->addr());
}

void CDispatchServiceMessageHandler::u2d_active_count_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const u2d_active_count_notify* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	pDispatchService->getOnlineCountMgr()->setUCActiveCount(sSessionInfo.nFromServiceID, pRequest->count());
}

void CDispatchServiceMessageHandler::l2d_validate_login_request_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const l2d_validate_login_request* pRequest)
{
	CDispatchService* pDispatchService = dynamic_cast<CDispatchService*>(pServiceBase);
	DebugAst(pDispatchService != nullptr);

	const std::string& szAccountName = pRequest->account_name();
	uint32_t nServerID = pRequest->server_id();

	PrintInfo("CDispatchServiceMessageHandler::validate_login_handler account_name: {} server_id: {}", szAccountName, nServerID);

	uint32_t nTblIndex = base::function_util::hash(szAccountName.c_str()) % pDispatchService->getAccountTblCount();

	// 先查询global_db是否有角色了
	std::string szSelectSQL = base::string_util::format("select player_id from {}{} where account_name = {?} and server_id = {}", _ACCOUNT_TBL_PREFIX, nTblIndex, nServerID);
	std::vector<std::string> vecArgs;
	vecArgs.push_back(szAccountName);

	std::shared_ptr<const google::protobuf::Message> pResponseMessage;
	if (this->sync_call(nTblIndex, szSelectSQL, &vecArgs, pResponseMessage) != eRRT_OK)
	{
		l2d_validate_login_response login_response_msg;
		login_response_msg.set_key("");
		login_response_msg.set_result(eEC_Login_QueryGlobalDbError);
		login_response_msg.set_gate_addr("");
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

		PrintInfo("CDispatchServiceMessageHandler::validate_login_handler select error account_name: {} server_id: {}", szAccountName, nServerID);
		return;
	}

	const proto::db::result_set* pResultset = dynamic_cast<const proto::db::result_set*>(pResponseMessage.get());
	if (nullptr == pResultset)
	{
		l2d_validate_login_response login_response_msg;
		login_response_msg.set_key("");
		login_response_msg.set_result(eEC_Login_QueryGlobalDbError);
		login_response_msg.set_gate_addr("");
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

		PrintInfo("CDispatchServiceMessageHandler::validate_login_handler nullptr == pResultset account_name: {} server_id: {}", szAccountName, nServerID);
		return;
	}

	DebugAst(pResultset->rows_size() <= 1);

	uint64_t nPlayerID = 0;
	uint32_t nUCServiceID = 0;
	if (pResultset->rows_size() == 1)
	{
		const proto::db::row& row = pResultset->rows(0);

		DebugAst(base::string_util::convert_to_value(row.value(0), nPlayerID));

		uint32_t nDBID = _GET_PLAYER_DB_ID(nPlayerID);
		nUCServiceID = nDBID + _UC_SERVICE_DELTA;
		DebugAst(nUCServiceID != 0);
	}
	else
	{
		nUCServiceID = pDispatchService->getOnlineCountMgr()->getSuitableUCID();
		if (nUCServiceID == 0)
		{
			l2d_validate_login_response login_response_msg;
			login_response_msg.set_key("");
			login_response_msg.set_result(eEC_Login_GetSuitableUCIDError);
			login_response_msg.set_gate_addr("");
			pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

			PrintInfo("CDispatchServiceMessageHandler::validate_login_handler nUCServiceID == 0 account_name: {} server_id: {}", szAccountName, nServerID);
			return;
		}

		uint32_t nDBID = nUCServiceID - _UC_SERVICE_DELTA;
		DebugAst(nDBID != 0);

		uint32_t nDbsID = nDBID + _GAME_DB_SERVICE_DELTA;
		DebugAst(nDbsID != 0);

		// 没有角色，取去找一个db，插入数据
		std::string szSPSQL = base::string_util::format("call sp_create_player_base({?}, {}, {})", nServerID, nDBID);
		std::vector<std::string> vecArgs;
		vecArgs.push_back(szAccountName);

		std::shared_ptr<const google::protobuf::Message> pResponseMessage;
		if (this->sync_call(nDbsID, nTblIndex, szSPSQL, &vecArgs, pResponseMessage) != eRRT_OK)
		{
			l2d_validate_login_response login_response_msg;
			login_response_msg.set_key("");
			login_response_msg.set_result(eEC_Login_InsertGameDbError);
			login_response_msg.set_gate_addr("");
			pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

			PrintInfo("CDispatchServiceMessageHandler::validate_login_handler call sp_create_player_base error account_name: {} server_id: {} db_id: {}", szAccountName, nServerID, nDBID);
			return;
		}

		const proto::db::result_set* pResultset = dynamic_cast<const proto::db::result_set*>(pResponseMessage.get());
		if (nullptr == pResultset)
		{
			l2d_validate_login_response login_response_msg;
			login_response_msg.set_key("");
			login_response_msg.set_result(eEC_Login_InsertGameDbError);
			login_response_msg.set_gate_addr("");
			pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

			PrintInfo("CDispatchServiceMessageHandler::validate_login_handler nullptr == pResultset account_name: {} server_id: {}", szAccountName, nServerID);
			return;
		}

		DebugAst(pResultset->rows_size() == 1);

		const proto::db::row& row = pResultset->rows(0);
		DebugAst(base::string_util::convert_to_value(row.value(0), nPlayerID));

		// 试着插入数据到账号表中，有可能失败
		std::string szInsertSQL = base::string_util::format("insert into {}{}(account_name, server_id, player_id, create_time) values({?}, {}, {}, {})", _ACCOUNT_TBL_PREFIX, nTblIndex, nServerID, nPlayerID, base::time_util::getGmtTime());
		vecArgs.clear();
		vecArgs.push_back(szAccountName);

		if (this->sync_call(nTblIndex, szInsertSQL, &vecArgs, pResponseMessage) != eRRT_OK)
		{
			l2d_validate_login_response login_response_msg;
			login_response_msg.set_key("");
			login_response_msg.set_result(eEC_Login_InsertGbloalDbError);
			login_response_msg.set_gate_addr("");
			pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

			PrintInfo("CDispatchServiceMessageHandler::validate_login_handler insert error account_name: {} server_id: {}", szAccountName, nServerID);
			return;
		}
	}

	uint32_t nGasID = pDispatchService->getOnlineCountMgr()->getSuitableGasID();
	
	d2u_player_login_request uc_request_msg;
	uc_request_msg.set_player_id(nPlayerID);
	uc_request_msg.set_gas_id(nGasID);

	// 去uc服务器占个位置
	std::shared_ptr<const d2u_player_login_response> pLoginResponseMessage;
	if (this->sync_invoke(nUCServiceID, &uc_request_msg, pLoginResponseMessage) != eRRT_OK)
	{
		l2d_validate_login_response login_response_msg;
		login_response_msg.set_key("");
		login_response_msg.set_result(eEC_Login_InvokeUCError);
		login_response_msg.set_gate_addr("");
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

		PrintInfo("CDispatchServiceMessageHandler::validate_login_handler invoke d2u_player_login_request error account_name: {} server_id: {} player_id: {}", szAccountName, nServerID, nPlayerID);
		return;
	}

	if (nullptr == pLoginResponseMessage)
	{
		l2d_validate_login_response login_response_msg;
		login_response_msg.set_key("");
		login_response_msg.set_result(eEC_Login_InvokeUCError);
		login_response_msg.set_gate_addr("");
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

		PrintInfo("CDispatchServiceMessageHandler::validate_login_handler nullptr == pLoginResponseMessage account_name: {} server_id: {} player_id: {}", szAccountName, nServerID, nPlayerID);
		return;
	}

	nGasID = pLoginResponseMessage->gas_id();
	if (nGasID == 0)
	{
		l2d_validate_login_response login_response_msg;
		login_response_msg.set_key("");
		login_response_msg.set_result(eEC_Login_InvokeUCError);
		login_response_msg.set_gate_addr("");
		pServiceBase->getServiceInvoker()->response(sSessionInfo, &login_response_msg);

		PrintInfo("CDispatchServiceMessageHandler::validate_login_handler nGasID == 0 account_name: {} server_id: {} player_id: {}", szAccountName, nServerID, nPlayerID);
		return;
	}

	uint32_t nGateID = pDispatchService->getOnlineCountMgr()->getSuitableGateID();
	std::string szToken = base::string_util::convert_to_str(base::CRandGen::getGlobalRand(1000000));

	d2g_player_token_notify gate_notify_msg;
	gate_notify_msg.set_player_id(nPlayerID);
	gate_notify_msg.set_gas_id(nGasID);
	gate_notify_msg.set_token(szToken);

	pDispatchService->getServiceInvoker()->send(nGateID, &gate_notify_msg);

	std::string szKey = base::string_util::format("{}|{}", nPlayerID, szToken);
	l2d_validate_login_response login_response_msg;
	login_response_msg.set_key(szKey);
	login_response_msg.set_result(eEC_Sucess);
	login_response_msg.set_gate_addr(pDispatchService->getOnlineCountMgr()->getGateAddr(nGateID));

	pDispatchService->getServiceInvoker()->response(sSessionInfo, &login_response_msg);
}