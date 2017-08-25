namespace core
{
	template<class T>
	void CServiceInvokeHolder::async_invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(eType, nID, pMessage, sFuture, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(eType, nID, pMessage, callback, this);
	}

	template<class T>
	uint32_t CServiceInvokeHolder::sync_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, eRRT_ERROR);

		return this->m_pServiceBase->getServiceInvoker()->sync_invoke(nServiceID, pMessage, pResponseMessage, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage, callback, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage, sFuture, this);
	}

	template<class T>
	uint32_t CServiceInvokeHolder::sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, eRRT_ERROR);

		return this->m_pServiceBase->getServiceInvoker()->sync_invoke(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage, pResponseMessage, this);
	}
}