#include "stdafx.h"
#include "db_service_invoke_holder.h"

#include "libBaseCommon/string_util.h"

#include "proto_src/select_command.pb.h"
#include "proto_src/delete_command.pb.h"
#include "proto_src/query_command.pb.h"
#include "proto_src/call_command.pb.h"
#include "proto_src/nop_command.pb.h"
#include "proto_src/flush_command.pb.h"
#include "proto_src/update_command.pb.h"
#include "proto_src/insert_command.pb.h"

namespace core
{
	/*
	这个文件的存在导致libCoreCommon必须依赖libprotobuf.a，在linux下libCoreCommon.so必须以动态加载的形式来链接，不然会出现protobuf全局符号覆盖问题
	*/
	CDbServiceInvokeHolder::CDbServiceInvokeHolder(CServiceBase* pServiceBase)
		: m_sServiceInvokeHolder(pServiceBase)
		, m_nDbServiceID(0)
	{
	}

	CDbServiceInvokeHolder::CDbServiceInvokeHolder(CServiceBase* pServiceBase, uint32_t nDbServiceID)
		: m_sServiceInvokeHolder(pServiceBase)
		, m_nDbServiceID(nDbServiceID)
	{
	}

	CDbServiceInvokeHolder::~CDbServiceInvokeHolder()
	{
		
	}

	bool CDbServiceInvokeHolder::update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

 		std::string szData = pMessage->SerializeAsString();
		proto::db::update_command command;
		command.set_message_name(pMessage->GetTypeName());
		command.set_message_content(szData);

 		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command);
	}

	bool CDbServiceInvokeHolder::remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName)
	{
		proto::db::delete_command command;
		command.set_id(nID);
		command.set_table_name(szTableName);

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command);
	}

	bool CDbServiceInvokeHolder::insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::string szData = pMessage->SerializeAsString();
		proto::db::insert_command command;
		command.set_message_name(pMessage->GetTypeName());
		command.set_message_content(szData);

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command);
	}

	bool CDbServiceInvokeHolder::call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg)
	{
		proto::db::call_command command;
		command.set_channel_id(nChannelID);
		command.set_sql(szSQL);
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.add_args(vecArg->at(i));
			}
		}

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command);
	}

	bool CDbServiceInvokeHolder::flush(uint32_t nDbServiceID, uint64_t nID, base::db::EFlushCacheType eType)
	{
		proto::db::flush_command command;
		command.set_id(nID);
		command.set_type(eType);

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command);
	}

	void CDbServiceInvokeHolder::async_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		proto::db::select_command command;
		command.set_id(nID);
		command.set_table_name(szTableName);

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback);
	}

	void CDbServiceInvokeHolder::async_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAst(pMessage != nullptr);

		std::string szData = pMessage->SerializeAsString();
		proto::db::update_command command;
		command.set_message_name(pMessage->GetTypeName());
		command.set_message_content(szData);

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback);
	}

	void CDbServiceInvokeHolder::async_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		proto::db::delete_command command;
		command.set_id(nID);
		command.set_table_name(szTableName);

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback);
	}

	void CDbServiceInvokeHolder::async_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAst(pMessage != nullptr);

		std::string szData = pMessage->SerializeAsString();
		proto::db::insert_command command;
		command.set_message_name(pMessage->GetTypeName());
		command.set_message_content(szData);

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback);
	}

	void CDbServiceInvokeHolder::async_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		proto::db::query_command command;
		command.set_channel_id(nChannelID);
		command.set_table_name(szTableName);
		command.set_where_clause(szWhereClause);
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.add_args(vecArg->at(i));
			}
		}

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback);
	}

	void CDbServiceInvokeHolder::async_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		proto::db::call_command command;
		command.set_channel_id(nChannelID);
		command.set_sql(szSQL);
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.add_args(vecArg->at(i));
			}
		}

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback);
	}

	void CDbServiceInvokeHolder::async_nop(uint32_t nDbServiceID, uint32_t nChannelID, const std::function<void(uint32_t)>& callback)
	{
		proto::db::nop_command command;
		command.set_channel_id(nChannelID);

		auto callback_ = [callback](const google::protobuf::Message* pMessage, uint32_t nErrorCode)
		{
			callback(nErrorCode);
		};

		this->m_sServiceInvokeHolder.async_invoke<google::protobuf::Message>(nDbServiceID, &command, callback_);
	}

	uint32_t CDbServiceInvokeHolder::sync_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		proto::db::select_command command;
		command.set_id(nID);
		command.set_table_name(szTableName);

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		DebugAstEx(pMessage != nullptr, eRRT_ERROR);

		std::string szData = pMessage->SerializeAsString();
		proto::db::update_command command;
		command.set_message_name(pMessage->GetTypeName());
		command.set_message_content(szData);

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		proto::db::delete_command command;
		command.set_id(nID);
		command.set_table_name(szTableName);

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		DebugAstEx(pMessage != nullptr, eRRT_ERROR);

		std::string szData = pMessage->SerializeAsString();
		proto::db::insert_command command;
		command.set_message_name(pMessage->GetTypeName());
		command.set_message_content(szData);

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		proto::db::query_command command;
		command.set_channel_id(nChannelID);
		command.set_table_name(szTableName);
		command.set_where_clause(szWhereClause);
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.add_args(vecArg->at(i));
			}
		}

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		proto::db::call_command command;
		command.set_channel_id(nChannelID);
		command.set_sql(szSQL);
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.add_args(vecArg->at(i));
			}
		}

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_nop(uint32_t nDbServiceID, uint32_t nChannelID)
	{
		proto::db::nop_command command;
		command.set_channel_id(nChannelID);

		std::shared_ptr<const google::protobuf::Message> pResponseMessage;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage);
	}

	bool CDbServiceInvokeHolder::update(const google::protobuf::Message* pMessage)
	{
		return this->update(this->m_nDbServiceID, pMessage);
	}

	bool CDbServiceInvokeHolder::remove(uint64_t nID, const std::string& szTableName)
	{
		return this->remove(this->m_nDbServiceID, nID, szTableName);
	}

	bool CDbServiceInvokeHolder::insert(const google::protobuf::Message* pMessage)
	{
		return this->insert(this->m_nDbServiceID, pMessage);
	}

	bool CDbServiceInvokeHolder::call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg)
	{
		return this->call(this->m_nDbServiceID, nChannelID, szSQL, vecArg);
	}

	bool CDbServiceInvokeHolder::flush(uint64_t nID, base::db::EFlushCacheType eType)
	{
		return this->flush(this->m_nDbServiceID, nID, eType);
	}

	void CDbServiceInvokeHolder::async_select(uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		this->async_select(this->m_nDbServiceID, nID, szTableName, callback);
	}

	void CDbServiceInvokeHolder::async_update(const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		this->async_update(this->m_nDbServiceID, pMessage, callback);
	}

	void CDbServiceInvokeHolder::async_remove(uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		this->async_remove(this->m_nDbServiceID, nID, szTableName, callback);
	}

	void CDbServiceInvokeHolder::async_insert(const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		this->async_insert(this->m_nDbServiceID, pMessage, callback);
	}

	void CDbServiceInvokeHolder::async_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		this->async_query(this->m_nDbServiceID, nChannelID, szTableName, szWhereClause, vecArg, callback);
	}

	void CDbServiceInvokeHolder::async_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		this->async_call(this->m_nDbServiceID, nChannelID, szSQL, vecArg, callback);
	}

	void CDbServiceInvokeHolder::async_nop(uint32_t nChannelID, const std::function<void(uint32_t)>& callback)
	{
		this->async_nop(this->m_nDbServiceID, nChannelID, callback);
	}

	uint32_t CDbServiceInvokeHolder::sync_select(uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		return this->sync_select(this->m_nDbServiceID, nID, szTableName, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_update(const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		return this->sync_update(this->m_nDbServiceID, pMessage, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_remove(uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		return this->sync_remove(this->m_nDbServiceID, nID, szTableName, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_insert(const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		return this->sync_insert(this->m_nDbServiceID, pMessage, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		return this->sync_query(this->m_nDbServiceID, nChannelID, szTableName, szWhereClause, vecArg, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		return this->sync_call(this->m_nDbServiceID, nChannelID, szSQL, vecArg, pResponseMessage);
	}

	uint32_t CDbServiceInvokeHolder::sync_nop(uint32_t nChannelID)
	{
		return this->sync_nop(this->m_nDbServiceID, nChannelID);
	}
}