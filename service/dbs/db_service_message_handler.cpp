#include "db_service_message_handler.h"
#include "db_service.h"

#include "libBaseDatabase/database.h"
#include "libCoreCommon/service_invoker.h"

CDbServiceMessageHandler::CDbServiceMessageHandler(CServiceBase* pServiceBase)
{
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::call_command_handler);
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::delete_command_handler);
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::flush_command_handler);
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::insert_command_handler);
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::nop_command_handler);
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::query_command_handler);
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::select_command_handler);
	register_service_message_handler(pServiceBase, this, &CDbServiceMessageHandler::update_command_handler);
}

CDbServiceMessageHandler::~CDbServiceMessageHandler()
{

}

void CDbServiceMessageHandler::call_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::call_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::delete_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::delete_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::flush_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::flush_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::insert_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::insert_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::nop_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::nop_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::query_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::query_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::select_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::select_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::update_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const proto::db::update_command* pRequest)
{
	this->normal_command_handler(pServiceBase, sSessionInfo, pRequest);
}

void CDbServiceMessageHandler::normal_command_handler(CServiceBase* pServiceBase, SSessionInfo sSessionInfo, const google::protobuf::Message* pRequest)
{
	CDbService* pDbService = dynamic_cast<CDbService*>(pServiceBase);
	DebugAst(pDbService != nullptr);

	base::db::query(pDbService->getDbID(), pRequest, [sSessionInfo, pServiceBase](const google::protobuf::Message* pResponse, uint32_t nErrorCode)
	{
		pServiceBase->getServiceInvoker()->response(sSessionInfo, pResponse, nErrorCode);
	});
}