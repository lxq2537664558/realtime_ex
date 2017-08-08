#pragma once
#include "libCoreCommon/service_message_registry.h"
#include "libCoreCommon/service_base.h"

#include "proto_src/gas_online_count_notify.pb.h"
#include "proto_src/gate_online_count_notify.pb.h"
#include "proto_src/validate_login_request.pb.h"
#include "proto_src/gate_addr_notify.pb.h"

using namespace core;

class CDispatchServiceMessageHandler :
	public CServiceMessageRegistry<CDispatchServiceMessageHandler>
{
	DEFEND_SERVICE_MESSAGE_FUNCTION(CDispatchServiceMessageHandler)

public:
	CDispatchServiceMessageHandler(CServiceBase* pServiceBase);
	~CDispatchServiceMessageHandler();

private:
	void		gas_online_count_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const gas_online_count_notify* pRequest);
	void		gate_online_count_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const gate_online_count_notify* pRequest);
	void		validate_login_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const validate_login_request* pRequest);
	void		gate_addr_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const gate_addr_notify* pRequest);
};