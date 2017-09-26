#pragma once
#include "libCoreCommon/db_service_invoke_holder.h"
#include "libCoreCommon/service_base.h"

#include "server_proto_src/s2d_online_count_notify.pb.h"
#include "server_proto_src/g2d_online_count_notify.pb.h"
#include "server_proto_src/l2d_validate_login_request.pb.h"
#include "server_proto_src/g2d_addr_notify.pb.h"
#include "server_proto_src/u2d_active_count_notify.pb.h"

using namespace core;

class CDispatchService;
class CDispatchServiceMessageHandler :
	public CDbServiceInvokeHolder,
	public CServiceInvokeHolder
{
public:
	CDispatchServiceMessageHandler(CDispatchService* pDispatchService);
	~CDispatchServiceMessageHandler();

private:
	void		s2d_online_count_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const s2d_online_count_notify* pRequest);
	void		g2d_online_count_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const g2d_online_count_notify* pRequest);
	void		l2d_validate_login_request_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const l2d_validate_login_request* pRequest);
	void		g2d_addr_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const g2d_addr_notify* pRequest);
	void		u2d_active_count_notify_handler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const u2d_active_count_notify* pRequest);
};