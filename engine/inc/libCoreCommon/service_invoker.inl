namespace core
{
	template<class T>
	void CServiceInvoker::async_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		DebugAst(pMessage != nullptr);
		
		auto pPromise = std::make_shared<CPromise<T>>();
		
		auto callback = [pPromise](std::shared_ptr<google::protobuf::Message> pResponseMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::dynamic_pointer_cast<T>(pResponseMessage), nErrorCode);
		};

		if (!this->invoke(nServiceID, 0, pMessage, 0, callback, pServiceInvokeHolder))
			pPromise->setValue(nullptr, eRRT_ERROR);

		sFuture = pPromise->getFuture();
	}

	template<class T>
	void CServiceInvoker::async_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		DebugAst(pMessage != nullptr);
		DebugAst(callback != nullptr);

		auto callback_ = [callback](std::shared_ptr<google::protobuf::Message> pResponseMessage, uint32_t nErrorCode)->void
		{
			callback(dynamic_cast<T*>(pResponseMessage.get()), nErrorCode);
		};

		if (!this->invoke(nServiceID, 0, pMessage, 0, callback_, pServiceInvokeHolder))
			callback(nullptr, eRRT_ERROR);
	}

	template<class T>
	uint32_t CServiceInvoker::sync_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		if (!this->invoke(nServiceID, 0, pMessage, coroutine::getCurrentID(), nullptr, pServiceInvokeHolder))
			return eRRT_ERROR;

		coroutine::yield();

		uint64_t nResponse = 0;

		DebugAstEx(coroutine::getLocalData(coroutine::getCurrentID(), "response", nResponse), eRRT_ERROR);
		coroutine::delLocalData(coroutine::getCurrentID(), "response");

		SSyncCallResultInfo* pSyncCallResultInfo = reinterpret_cast<SSyncCallResultInfo*>(nResponse);
		pResponseMessage = std::dynamic_pointer_cast<T>(pSyncCallResultInfo->pMessage);

		uint8_t nResult = pSyncCallResultInfo->nResult;

		SAFE_DELETE(pSyncCallResultInfo);

		return nResult;
	}

	template<class T>
	void CServiceInvoker::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAst(pServiceSelector != nullptr);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);

		this->async_invoke(nServiceID, pMessage, callback);
	}

	template<class T>
	void CServiceInvoker::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAst(pServiceSelector != nullptr);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);

		this->async_invoke(nServiceID, pMessage, sFuture);
	}

	template<class T>
	uint32_t CServiceInvoker::sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage, CServiceInvokeHolder* pServiceInvokeHolder /*= nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAstEx(pServiceSelector != nullptr, eRRT_ERROR);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);

		return this->sync_invoke(nServiceID, pMessage, pResponseMessage);
	}

	template<class T>
	void CServiceInvoker::async_invoke_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder /*= nullptr*/)
	{
		DebugAst(pMessage != nullptr);
		DebugAst(callback != nullptr);

		auto callback_ = [callback](std::shared_ptr<google::protobuf::Message> pResponseMessage, uint32_t nErrorCode)->void
		{
			callback(dynamic_cast<T*>(pResponseMessage.get()), nErrorCode);
		};

		if (!this->invoke(nServiceID, nActorID, pMessage, 0, callback_, pServiceInvokeHolder))
			callback(nullptr, eRRT_ERROR);
	}

	template<class T>
	void CServiceInvoker::async_invoke_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder /*= nullptr*/)
	{
		DebugAst(pMessage != nullptr);

		auto pPromise = std::make_shared<CPromise<T>>();

		auto callback = [pPromise](std::shared_ptr<google::protobuf::Message> pResponseMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::dynamic_pointer_cast<T>(pResponseMessage), nErrorCode);
		};

		if (!this->invoke(nServiceID, nActorID, pMessage, 0, callback, pServiceInvokeHolder))
			pPromise->setValue(nullptr, eRRT_ERROR);

		sFuture = pPromise->getFuture();
	}
}