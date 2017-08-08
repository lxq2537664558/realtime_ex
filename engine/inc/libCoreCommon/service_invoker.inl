namespace core
{
	template<class T>
	void CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		DebugAst(pMessage != nullptr);
		
		auto pPromise = std::make_shared<CPromise<T>>();
		
		auto callback = [pPromise](std::shared_ptr<google::protobuf::Message> pResponseMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::dynamic_pointer_cast<T>(pResponseMessage), nErrorCode);
		};

		if (!this->invoke(eType, nID, pMessage, callback, pServiceInvokeHolder))
			pPromise->setValue(nullptr, eRRT_ERROR);

		sFuture = pPromise->getFuture();
	}

	template<class T>
	void CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		DebugAst(pMessage != nullptr);
		DebugAst(callback != nullptr);

		auto callback_ = [callback](std::shared_ptr<google::protobuf::Message> pResponseMessage, uint32_t nErrorCode)->void
		{
			callback(dynamic_cast<T*>(pResponseMessage.get()), nErrorCode);
		};

		if (!this->invoke(eType, nID, pMessage, callback_, pServiceInvokeHolder))
			callback(nullptr, eRRT_ERROR);
	}

	template<class T>
	void CServiceInvoker::async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(szServiceSelectorType);
		DebugAst(pServiceSelector != nullptr);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, szServiceSelectorType, nServiceSelectorContext);

		this->async_call(eMTT_Service, nServiceID, pMessage, callback);
	}

	template<class T>
	void CServiceInvoker::async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(szServiceSelectorType);
		DebugAst(pServiceSelector != nullptr);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, szServiceSelectorType, nServiceSelectorContext);

		this->async_call(eMTT_Service, nServiceID, pMessage, sFuture);
	}
}