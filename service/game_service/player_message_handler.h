#pragma once
#include "libCoreCommon/service_base.h"

#include "client_proto_src/c2s_update_name_request.pb.h"
#include "client_proto_src/c2s_gm_command_request.pb.h"

using namespace core;

class CPlayerMessageHandler
{
public:
	CPlayerMessageHandler(CServiceBase* pServiceBase);
	~CPlayerMessageHandler();

private:
	void c2s_update_name_request_handler(CServiceBase* pServiceBase, SClientSessionInfo sClientSessionInfo, const c2s_update_name_request* pRequest);
	void c2s_gm_command_request_handler(CServiceBase* pServiceBase, SClientSessionInfo sClientSessionInfo, const c2s_gm_command_request* pRequest);
};