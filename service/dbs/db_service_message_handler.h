#pragma once
#include "libCoreCommon/service_base.h"
#include "libCoreCommon/db_native_message.h"

#include "google/protobuf/message.h"

using namespace core;

class CDbServiceMessageHandler
{
public:
	CDbServiceMessageHandler(CServiceBase* pServiceBase);
	~CDbServiceMessageHandler();

private:
	void call_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const call_command* pRequest);
	void delete_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const delete_command* pRequest);
	void flush_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const flush_command* pRequest);
	void insert_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const insert_command* pRequest);
	void nop_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const nop_command* pRequest);
	void query_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const query_command* pRequest);
	void select_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const select_command* pRequest);
	void update_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const update_command* pRequest);

	void normal_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const google::protobuf::Message* pRequest);
};