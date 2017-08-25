#include "db_command_handler_proxy.h"
#include "db_connection.h"
#include "db_command_call_handler.h"
#include "db_command_delete_handler.h"
#include "db_command_insert_handler.h"
#include "db_command_nop_handler.h"
#include "db_command_query_handler.h"
#include "db_command_select_handler.h"
#include "db_command_update_handler.h"
#include "db_protobuf.h"

namespace base
{
	CDbCommandHandlerProxy::CDbCommandHandlerProxy()
	{
		this->m_mapDbCommandHandler[db::eDBCT_Call] = new CDbCommandCallHandler();
		this->m_mapDbCommandHandler[db::eDBCT_Delete] = new CDbCommandDeleteHandler();
		this->m_mapDbCommandHandler[db::eDBCT_Insert] = new CDbCommandInsertHandler();
		this->m_mapDbCommandHandler[db::eDBCT_Query] = new CDbCommandQueryHandler();
		this->m_mapDbCommandHandler[db::eDBCT_Select] = new CDbCommandSelectHandler();
		this->m_mapDbCommandHandler[db::eDBCT_Update] = new CDbCommandUpdateHandler();
		this->m_mapDbCommandHandler[db::eDBCT_Nop] = new CDbCommandNOPHandler();
	}

	CDbCommandHandlerProxy::~CDbCommandHandlerProxy()
	{
		for (auto iter = this->m_mapDbCommandHandler.begin(); iter != this->m_mapDbCommandHandler.end(); ++iter)
		{
			delete iter->second;
		}
	}

	bool CDbCommandHandlerProxy::init()
	{
		return true;
	}

	void CDbCommandHandlerProxy::onConnect(CDbConnection* pDbConnection)
	{
		pDbConnection->autoCommit(true);

		for (auto iter = this->m_mapDbCommandHandler.begin(); iter != this->m_mapDbCommandHandler.end(); ++iter)
		{
			auto& pDbCommandHandler = iter->second;
			if (pDbCommandHandler == nullptr)
				continue;

			pDbCommandHandler->onConnect(pDbConnection);
		}
	}

	void CDbCommandHandlerProxy::onDisconnect()
	{
		for (auto iter = this->m_mapDbCommandHandler.begin(); iter != this->m_mapDbCommandHandler.end(); ++iter)
		{
			CDbCommandHandler* pDbCommandHandler = iter->second;
			if (pDbCommandHandler == nullptr)
				continue;

			pDbCommandHandler->onDisconnect();
		}
	}

	uint32_t CDbCommandHandlerProxy::onDbCommand(uint32_t nType, google::protobuf::Message* pRequestMessage, std::shared_ptr<google::protobuf::Message>* pResponseMessage)
	{
		auto iter = this->m_mapDbCommandHandler.find(nType);
		if (iter == this->m_mapDbCommandHandler.end())
			return db::eDBRC_Unknown;

		CDbCommandHandler* pDbCommandHandler = iter->second;
		if (pDbCommandHandler == nullptr)
			return db::eDBRC_Unknown;

		return pDbCommandHandler->onDbCommand(pRequestMessage, pResponseMessage);
	}
}