namespace core
{
	bool CDbServiceInvokeHolder::service_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

 		std::string szData = pMessage->SerializeAsString();
		update_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = 0;
 		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, &sInvokeOption);
	}

	bool CDbServiceInvokeHolder::service_remove(uint32_t nDbServiceID, uint64_t nPrimaryID, const std::string& szTableName)
	{
		delete_command command;
		command.primary_id = nPrimaryID;
		command.table_name = szTableName;

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = 0;
		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, &sInvokeOption);
	}

	bool CDbServiceInvokeHolder::service_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::string szData = pMessage->SerializeAsString();
		insert_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = 0;
		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, &sInvokeOption);
	}

	bool CDbServiceInvokeHolder::service_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg)
	{
		call_command command;
		command.channel_id = nChannelID;
		command.sql = szSQL;
		if (vecArg != nullptr)
		{
			command.args.reserve(vecArg->size());
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = 0;
		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, &sInvokeOption);
	}

	bool CDbServiceInvokeHolder::service_flush(uint32_t nDbServiceID, uint64_t nPrimaryID, base::db::EFlushCacheType eType)
	{
		flush_command command;
		command.primary_id = nPrimaryID;
		command.type = (uint32_t)eType;

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = 0;
		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, &sInvokeOption);
	}

	void CDbServiceInvokeHolder::service_async_select(uint32_t nDbServiceID, uint64_t nPrimaryID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		select_command command;
		command.primary_id = nPrimaryID;
		command.table_name = szTableName;

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, &sInvokeOption);
	}

	void CDbServiceInvokeHolder::service_async_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		DebugAst(pMessage != nullptr);

		std::string szData = pMessage->SerializeAsString();
		update_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		auto callback_ = [callback](const google::protobuf::Message*, uint32_t nErrorCode)->void
		{
			callback(nErrorCode);
		};

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		this->m_sServiceInvokeHolder.async_invoke<google::protobuf::Message>(nDbServiceID, &command, callback_, &sInvokeOption);
	}

	void CDbServiceInvokeHolder::service_async_remove(uint32_t nDbServiceID, uint64_t nPrimaryID, const std::string& szTableName, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		delete_command command;
		command.primary_id = nPrimaryID;
		command.table_name = szTableName;

		auto callback_ = [callback](const google::protobuf::Message*, uint32_t nErrorCode)->void
		{
			callback(nErrorCode);
		};

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		this->m_sServiceInvokeHolder.async_invoke<google::protobuf::Message>(nDbServiceID, &command, callback_, &sInvokeOption);
	}

	void CDbServiceInvokeHolder::service_async_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		DebugAst(pMessage != nullptr);

		std::string szData = pMessage->SerializeAsString();
		insert_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		auto callback_ = [callback](const google::protobuf::Message*, uint32_t nErrorCode)->void
		{
			callback(nErrorCode);
		};

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		this->m_sServiceInvokeHolder.async_invoke<google::protobuf::Message>(nDbServiceID, &command, callback_, &sInvokeOption);
	}

	void CDbServiceInvokeHolder::service_async_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		query_command command;
		command.channel_id = nChannelID;
		command.table_name = szTableName;
		command.where_clause = szWhereClause;
		if (vecArg != nullptr)
		{
			command.args.reserve(vecArg->size());
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, &sInvokeOption);
	}

	void CDbServiceInvokeHolder::service_async_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		call_command command;
		command.channel_id = nChannelID;
		command.sql = szSQL;
		if (vecArg != nullptr)
		{
			command.args.reserve(vecArg->size());
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, &sInvokeOption);
	}

	void CDbServiceInvokeHolder::service_async_nop(uint32_t nDbServiceID, uint32_t nChannelID, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		nop_command command;
		command.channel_id = nChannelID;

		auto callback_ = [callback](const google::protobuf::Message* pMessage, uint32_t nErrorCode)
		{
			callback(nErrorCode);
		};

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		this->m_sServiceInvokeHolder.async_invoke<google::protobuf::Message>(nDbServiceID, &command, callback_, &sInvokeOption);
	}

	uint32_t CDbServiceInvokeHolder::service_sync_select(uint32_t nDbServiceID, uint64_t nPrimaryID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage, uint32_t nTimeout/* = 0*/)
	{
		select_command command;
		command.primary_id = nPrimaryID;
		command.table_name = szTableName;

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, &sInvokeOption);
	}

	uint32_t CDbServiceInvokeHolder::service_sync_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, uint32_t nTimeout/* = 0*/)
	{
		DebugAstEx(pMessage != nullptr, eRRT_ERROR);

		std::string szData = pMessage->SerializeAsString();
		update_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		std::shared_ptr<const google::protobuf::Message> pResponseMessage;
		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, &sInvokeOption);
	}

	uint32_t CDbServiceInvokeHolder::service_sync_remove(uint32_t nDbServiceID, uint64_t nPrimaryID, const std::string& szTableName, uint32_t nTimeout/* = 0*/)
	{
		delete_command command;
		command.primary_id = nPrimaryID;
		command.table_name = szTableName;

		std::shared_ptr<const google::protobuf::Message> pResponseMessage;
		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, &sInvokeOption);
	}

	uint32_t CDbServiceInvokeHolder::service_sync_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, uint32_t nTimeout/* = 0*/)
	{
		DebugAstEx(pMessage != nullptr, eRRT_ERROR);

		std::string szData = pMessage->SerializeAsString();
		insert_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		std::shared_ptr<const google::protobuf::Message> pResponseMessage;
		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, &sInvokeOption);
	}

	uint32_t CDbServiceInvokeHolder::service_sync_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage, uint32_t nTimeout/* = 0*/)
	{
		query_command command;
		command.channel_id = nChannelID;
		command.table_name = szTableName;
		command.where_clause = szWhereClause;
		if (vecArg != nullptr)
		{
			command.args.reserve(vecArg->size());
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, &sInvokeOption);
	}

	uint32_t CDbServiceInvokeHolder::service_sync_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage, uint32_t nTimeout/* = 0*/)
	{
		call_command command;
		command.channel_id = nChannelID;
		command.sql = szSQL;
		if (vecArg != nullptr)
		{
			command.args.reserve(vecArg->size());
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, &sInvokeOption);
	}

	uint32_t CDbServiceInvokeHolder::service_sync_nop(uint32_t nDbServiceID, uint32_t nChannelID, uint32_t nTimeout/* = 0*/)
	{
		nop_command command;
		command.channel_id = nChannelID;

		std::shared_ptr<const google::protobuf::Message> pResponseMessage;
		SInvokeOption sInvokeOption;
		sInvokeOption.nSerializerType = eMST_Native;
		sInvokeOption.nTimeout = nTimeout;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, &sInvokeOption);
	}

	bool CDbServiceInvokeHolder::update(const google::protobuf::Message* pMessage)
	{
		return this->service_update(this->m_nDbServiceID, pMessage);
	}

	bool CDbServiceInvokeHolder::remove(uint64_t nPrimaryID, const std::string& szTableName)
	{
		return this->service_remove(this->m_nDbServiceID, nPrimaryID, szTableName);
	}

	bool CDbServiceInvokeHolder::insert(const google::protobuf::Message* pMessage)
	{
		return this->service_insert(this->m_nDbServiceID, pMessage);
	}

	bool CDbServiceInvokeHolder::call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg)
	{
		return this->service_call(this->m_nDbServiceID, nChannelID, szSQL, vecArg);
	}

	bool CDbServiceInvokeHolder::flush(uint64_t nPrimaryID, base::db::EFlushCacheType eType)
	{
		return this->service_flush(this->m_nDbServiceID, nPrimaryID, eType);
	}

	void CDbServiceInvokeHolder::async_select(uint64_t nPrimaryID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		this->service_async_select(this->m_nDbServiceID, nPrimaryID, szTableName, callback, nTimeout);
	}

	void CDbServiceInvokeHolder::async_update(const google::protobuf::Message* pMessage, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		this->service_async_update(this->m_nDbServiceID, pMessage, callback, nTimeout);
	}

	void CDbServiceInvokeHolder::async_remove(uint64_t nPrimaryID, const std::string& szTableName, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		this->service_async_remove(this->m_nDbServiceID, nPrimaryID, szTableName, callback, nTimeout);
	}

	void CDbServiceInvokeHolder::async_insert(const google::protobuf::Message* pMessage, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		this->service_async_insert(this->m_nDbServiceID, pMessage, callback, nTimeout);
	}

	void CDbServiceInvokeHolder::async_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		this->service_async_query(this->m_nDbServiceID, nChannelID, szTableName, szWhereClause, vecArg, callback, nTimeout);
	}

	void CDbServiceInvokeHolder::async_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		this->service_async_call(this->m_nDbServiceID, nChannelID, szSQL, vecArg, callback, nTimeout);
	}

	void CDbServiceInvokeHolder::async_nop(uint32_t nChannelID, const std::function<void(uint32_t)>& callback, uint32_t nTimeout/* = 0*/)
	{
		this->service_async_nop(this->m_nDbServiceID, nChannelID, callback, nTimeout);
	}

	uint32_t CDbServiceInvokeHolder::sync_select(uint64_t nPrimaryID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage, uint32_t nTimeout/* = 0*/)
	{
		return this->service_sync_select(this->m_nDbServiceID, nPrimaryID, szTableName, pResponseMessage, nTimeout);
	}

	uint32_t CDbServiceInvokeHolder::sync_update(const google::protobuf::Message* pMessage, uint32_t nTimeout/* = 0*/)
	{
		return this->service_sync_update(this->m_nDbServiceID, pMessage, nTimeout);
	}

	uint32_t CDbServiceInvokeHolder::sync_remove(uint64_t nPrimaryID, const std::string& szTableName, uint32_t nTimeout/* = 0*/)
	{
		return this->service_sync_remove(this->m_nDbServiceID, nPrimaryID, szTableName, nTimeout);
	}

	uint32_t CDbServiceInvokeHolder::sync_insert(const google::protobuf::Message* pMessage, uint32_t nTimeout/* = 0*/)
	{
		return this->service_sync_insert(this->m_nDbServiceID, pMessage, nTimeout);
	}

	uint32_t CDbServiceInvokeHolder::sync_query(uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage, uint32_t nTimeout/* = 0*/)
	{
		return this->service_sync_query(this->m_nDbServiceID, nChannelID, szTableName, szWhereClause, vecArg, pResponseMessage, nTimeout);
	}

	uint32_t CDbServiceInvokeHolder::sync_call(uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage, uint32_t nTimeout/* = 0*/)
	{
		return this->service_sync_call(this->m_nDbServiceID, nChannelID, szSQL, vecArg, pResponseMessage, nTimeout);
	}

	uint32_t CDbServiceInvokeHolder::sync_nop(uint32_t nChannelID, uint32_t nTimeout/* = 0*/)
	{
		return this->service_sync_nop(this->m_nDbServiceID, nChannelID, nTimeout);
	}
}