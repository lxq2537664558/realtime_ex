namespace core
{
	template<class T>
	void CActorInvokeHolder::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback)
	{
		DebugAst(this->m_pActorBase != nullptr);

		this->m_pActorBase->async_call(eType, nID, pMessage, callback, this);
	}

	template<class T>
	void CActorInvokeHolder::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAst(this->m_pActorBase != nullptr);

		this->m_pActorBase->async_call(eType, nID, pMessage, sFuture, this);
	}

	template<class T>
	uint32_t CActorInvokeHolder::sync_call(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage)
	{
		DebugAstEx(this->m_pActorBase != nullptr, false);

		return this->m_pActorBase->sync_call(nServiceID, pMessage, pResponseMessage, this);
	}

	template<class T>
	void CActorInvokeHolder::async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback)
	{
		DebugAst(this->m_pActorBase != nullptr);

		this->m_pActorBase->async_call(szServiceType, szServiceSelectorType, nServiceSelectorContext, pMessage, callback, this);
	}

	template<class T>
	void CActorInvokeHolder::async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAst(this->m_pActorBase != nullptr);

		this->m_pActorBase->async_call(szServiceType, szServiceSelectorType, nServiceSelectorContext, pMessage, sFuture, this);
	}
}