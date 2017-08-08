namespace core
{
	template<class T>
	void CServiceInvokeHolder::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_call(eType, nID, pMessage, sFuture, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_call(eType, nID, pMessage, callback, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_call(szServiceType, szServiceSelectorType, nServiceSelectorContext, pMessage, callback, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_call(szServiceType, szServiceSelectorType, nServiceSelectorContext, pMessage, sFuture, this);
	}
}