namespace core
{
	bool CDbServiceInvokeHolder::update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

 		std::string szData = pMessage->SerializeAsString();
		update_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

 		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, eMST_Native);
	}

	bool CDbServiceInvokeHolder::remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName)
	{
		delete_command command;
		command.id = nID;
		command.table_name = szTableName;

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, eMST_Native);
	}

	bool CDbServiceInvokeHolder::insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, false);

		std::string szData = pMessage->SerializeAsString();
		insert_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, eMST_Native);
	}

	bool CDbServiceInvokeHolder::call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg)
	{
		call_command command;
		command.channel_id = nChannelID;
		command.sql = szSQL;
		command.args.reserve(vecArg->size());
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, eMST_Native);
	}

	bool CDbServiceInvokeHolder::flush(uint32_t nDbServiceID, uint64_t nID, base::db::EFlushCacheType eType)
	{
		flush_command command;
		command.id = nID;
		command.type = (uint32_t)eType;

		return this->m_sServiceInvokeHolder.send(nDbServiceID, &command, eMST_Native);
	}

	void CDbServiceInvokeHolder::async_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		select_command command;
		command.id = nID;
		command.table_name = szTableName;

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, eMST_Native);
	}

	void CDbServiceInvokeHolder::async_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAst(pMessage != nullptr);

		std::string szData = pMessage->SerializeAsString();
		update_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, eMST_Native);
	}

	void CDbServiceInvokeHolder::async_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		delete_command command;
		command.id = nID;
		command.table_name = szTableName;

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, eMST_Native);
	}

	void CDbServiceInvokeHolder::async_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAst(pMessage != nullptr);

		std::string szData = pMessage->SerializeAsString();
		insert_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, eMST_Native);
	}

	void CDbServiceInvokeHolder::async_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		query_command command;
		command.channel_id = nChannelID;
		command.table_name = szTableName;
		command.where_clause = szWhereClause;
		command.args.reserve(vecArg->size());
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, eMST_Native);
	}

	void CDbServiceInvokeHolder::async_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		call_command command;
		command.channel_id = nChannelID;
		command.sql = szSQL;
		command.args.reserve(vecArg->size());
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		this->m_sServiceInvokeHolder.async_invoke(nDbServiceID, &command, callback, eMST_Native);
	}

	void CDbServiceInvokeHolder::async_nop(uint32_t nDbServiceID, uint32_t nChannelID, const std::function<void(uint32_t)>& callback)
	{
		nop_command command;
		command.channel_id = nChannelID;

		auto callback_ = [callback](const google::protobuf::Message* pMessage, uint32_t nErrorCode)
		{
			callback(nErrorCode);
		};

		this->m_sServiceInvokeHolder.async_invoke<google::protobuf::Message>(nDbServiceID, &command, callback_, eMST_Native);
	}

	uint32_t CDbServiceInvokeHolder::sync_select(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		select_command command;
		command.id = nID;
		command.table_name = szTableName;

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, eMST_Native);
	}

	uint32_t CDbServiceInvokeHolder::sync_update(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		DebugAstEx(pMessage != nullptr, eRRT_ERROR);

		std::string szData = pMessage->SerializeAsString();
		update_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, eMST_Native);
	}

	uint32_t CDbServiceInvokeHolder::sync_remove(uint32_t nDbServiceID, uint64_t nID, const std::string& szTableName, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		delete_command command;
		command.id = nID;
		command.table_name = szTableName;

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, eMST_Native);
	}

	uint32_t CDbServiceInvokeHolder::sync_insert(uint32_t nDbServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		DebugAstEx(pMessage != nullptr, eRRT_ERROR);

		std::string szData = pMessage->SerializeAsString();
		insert_command command;
		command.message_name = pMessage->GetTypeName();
		command.message_content = std::move(szData);

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, eMST_Native);
	}

	uint32_t CDbServiceInvokeHolder::sync_query(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szTableName, const std::string& szWhereClause, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		query_command command;
		command.channel_id = nChannelID;
		command.table_name = szTableName;
		command.where_clause = szWhereClause;
		command.args.reserve(vecArg->size());
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, eMST_Native);
	}

	uint32_t CDbServiceInvokeHolder::sync_call(uint32_t nDbServiceID, uint32_t nChannelID, const std::string& szSQL, const std::vector<std::string>* vecArg, std::shared_ptr<const google::protobuf::Message>& pResponseMessage)
	{
		call_command command;
		command.channel_id = nChannelID;
		command.sql = szSQL;
		command.args.reserve(vecArg->size());
		if (vecArg != nullptr)
		{
			for (size_t i = 0; i < vecArg->size(); ++i)
			{
				command.args.push_back(vecArg->at(i));
			}
		}

		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, eMST_Native);
	}

	uint32_t CDbServiceInvokeHolder::sync_nop(uint32_t nDbServiceID, uint32_t nChannelID)
	{
		nop_command command;
		command.channel_id = nChannelID;

		std::shared_ptr<const google::protobuf::Message> pResponseMessage;
		return this->m_sServiceInvokeHolder.sync_invoke(nDbServiceID, &command, pResponseMessage, eMST_Native);
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