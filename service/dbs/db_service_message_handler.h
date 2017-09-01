#pragma once
#include "libCoreCommon/service_base.h"

#include "proto_src/call_command.pb.h"
#include "proto_src/delete_command.pb.h"
#include "proto_src/nop_command.pb.h"
#include "proto_src/flush_command.pb.h"
#include "proto_src/insert_command.pb.h"
#include "proto_src/query_command.pb.h"
#include "proto_src/select_command.pb.h"
#include "proto_src/update_command.pb.h"

using namespace core;

class CDbServiceMessageHandler
{
public:
	CDbServiceMessageHandler(CServiceBase* pServiceBase);
	~CDbServiceMessageHandler();

private:
	void call_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::call_command* pRequest);
	void delete_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::delete_command* pRequest);
	void flush_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::flush_command* pRequest);
	void insert_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::insert_command* pRequest);
	void nop_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::nop_command* pRequest);
	void query_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::query_command* pRequest);
	void select_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::select_command* pRequest);
	void update_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::update_command* pRequest);

	void normal_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const google::protobuf::Message* pRequest);
};