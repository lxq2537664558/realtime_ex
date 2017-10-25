#include "db_service_message_handler.h"
#include "db_service.h"

#include "libBaseDatabase/database.h"

#include "libCoreCommon/service_invoker.h"
#include "libCoreCommon/register_message_util.h"

#include "proto_src/call_command.pb.h"
#include "proto_src/delete_command.pb.h"
#include "proto_src/flush_command.pb.h"
#include "proto_src/insert_command.pb.h"
#include "proto_src/nop_command.pb.h"
#include "proto_src/query_command.pb.h"
#include "proto_src/select_command.pb.h"
#include "proto_src/update_command.pb.h"

CDbServiceMessageHandler::CDbServiceMessageHandler(CServiceBase* pServiceBase)
{
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::select_command_handler);
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::update_command_handler);
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::delete_command_handler);
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::insert_command_handler);
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::call_command_handler);
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::query_command_handler);
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::flush_command_handler);
	register_native_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::nop_command_handler);
}

CDbServiceMessageHandler::~CDbServiceMessageHandler()
{

}

void CDbServiceMessageHandler::call_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const call_command* pCommand)
{
	proto::db::call_command pb_command;
	pb_command.set_channel_id(pCommand->channel_id);
	pb_command.set_sql(pCommand->sql);
	for (size_t i = 0; i < pCommand->args.size(); ++i)
	{
		pb_command.add_args(pCommand->args[i]);
	}

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::delete_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const delete_command* pCommand)
{
	proto::db::delete_command pb_command;
	pb_command.set_id(pCommand->primary_id);
	pb_command.set_table_name(pCommand->table_name);

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::flush_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const flush_command* pCommand)
{
	proto::db::flush_command pb_command;
	pb_command.set_id(pCommand->primary_id);
	pb_command.set_type(pCommand->type);

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::insert_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const insert_command* pCommand)
{
	proto::db::insert_command pb_command;
	pb_command.set_message_name(pCommand->message_name);
	pb_command.set_message_content(pCommand->message_content);

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::nop_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const nop_command* pCommand)
{
	proto::db::nop_command pb_command;
	pb_command.set_channel_id(pCommand->channel_id);

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::query_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const query_command* pCommand)
{
	proto::db::query_command pb_command;
	pb_command.set_channel_id(pCommand->channel_id);
	pb_command.set_table_name(pCommand->table_name);
	pb_command.set_where_clause(pCommand->where_clause);
	for (size_t i = 0; i < pCommand->args.size(); ++i)
	{
		pb_command.add_args(pCommand->args[i]);
	}

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::select_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const select_command* pCommand)
{
	proto::db::select_command pb_command;
	pb_command.set_id(pCommand->primary_id);
	pb_command.set_table_name(pCommand->table_name);

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::update_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const update_command* pCommand)
{
	proto::db::update_command pb_command;
	pb_command.set_message_name(pCommand->message_name);
	pb_command.set_message_content(pCommand->message_content);

	this->normal_command_handler(pServiceBase, sSessionInfo, &pb_command);
}

void CDbServiceMessageHandler::normal_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const google::protobuf::Message* pRequest)
{
	CDbService* pDbService = dynamic_cast<CDbService*>(pServiceBase);
	DebugAst(pDbService != nullptr);

	base::db::query(pDbService->getDbID(), pRequest, [sSessionInfo, pServiceBase](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
	{
		if (sSessionInfo.nSessionID != 0)
			pServiceBase->getServiceInvoker()->response(sSessionInfo, pResponse, nErrorCode);
	});
}