namespace core
{
	template<class T>
	void CServiceInvoker::async_invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, const SInvokeOption* pInvokeOption, CServiceInvokeHolder* pServiceInvokeHolder)
	{
		DebugAst(pMessage != nullptr);

		auto pPromise = std::make_shared<CPromise<T>>();

		auto callback = [pPromise](std::shared_ptr<void> pResponseMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::static_pointer_cast<T>(pResponseMessage), nErrorCode);
		};

		if (!this->invoke(bCheckHealth, nServiceID, pMessage, 0, callback, pInvokeOption, pServiceInvokeHolder))
			pPromise->setValue(nullptr, eRRT_ERROR);

		sFuture = pPromise->getFuture();
	}

	template<class T>
	void CServiceInvoker::async_invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, const SInvokeOption* pInvokeOption, CServiceInvokeHolder* pServiceInvokeHolder)
	{
		DebugAst(pMessage != nullptr);
		DebugAst(callback != nullptr);

		auto callback_ = [callback](std::shared_ptr<void> pResponseMessage, uint32_t nErrorCode)->void
		{
			callback(static_cast<T*>(pResponseMessage.get()), nErrorCode);
		};

		if (!this->invoke(bCheckHealth, nServiceID, pMessage, 0, callback_, pInvokeOption, pServiceInvokeHolder))
			callback(nullptr, eRRT_ERROR);
	}

	template<class T>
	uint32_t CServiceInvoker::sync_invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, const SInvokeOption* pInvokeOption, CServiceInvokeHolder* pServiceInvokeHolder)
	{
		struct SSyncCallResultInfo
		{
			uint32_t	nResult;
			std::shared_ptr<void>
						pMessage;
		};

		DebugAstEx(pMessage != nullptr, eRRT_ERROR);
		uint64_t nCoroutineID = coroutine::getCurrentID();
		auto callback_ = [nCoroutineID](std::shared_ptr<void> pResponseMessage, uint32_t nErrorCode)->void
		{
			SSyncCallResultInfo* pSyncCallResultInfo = new SSyncCallResultInfo();
			pSyncCallResultInfo->nResult = nErrorCode;
			pSyncCallResultInfo->pMessage = pResponseMessage;
			coroutine::setLocalData(nCoroutineID, "response", reinterpret_cast<uint64_t>(pSyncCallResultInfo));

			coroutine::resume(nCoroutineID, 0);
		};

		if (coroutine::getCurrentID() == 0)
		{
			PrintWarning("root coroutine can't sync_invoke");
			return eRRT_ERROR;
		}

		if (!this->invoke(bCheckHealth, nServiceID, pMessage, nCoroutineID, callback_, pInvokeOption, pServiceInvokeHolder))
			return eRRT_ERROR;
		
		// 不用怕上面的resume先执行（invoke跟yield不是原子的），因为在yield完之前response消息是不可能被调度到的
		coroutine::yield();

		uint64_t nResponse = 0;

		DebugAstEx(coroutine::getLocalData(coroutine::getCurrentID(), "response", nResponse), eRRT_ERROR);
		coroutine::delLocalData(coroutine::getCurrentID(), "response");

		SSyncCallResultInfo* pSyncCallResultInfo = reinterpret_cast<SSyncCallResultInfo*>(nResponse);
		pResponseMessage = std::static_pointer_cast<T>(pSyncCallResultInfo->pMessage);

		uint32_t nResult = pSyncCallResultInfo->nResult;

		SAFE_DELETE(pSyncCallResultInfo);

		return nResult;
	}

	template<class T>
	void CServiceInvoker::async_invoke(uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, const SInvokeOption* pInvokeOption/* = nullptr */, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		this->async_invoke(true, nServiceID, pMessage, sFuture, pInvokeOption, pServiceInvokeHolder);
	}

	template<class T>
	void CServiceInvoker::async_invoke(uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, const SInvokeOption* pInvokeOption/* = nullptr */, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		this->async_invoke(true, nServiceID, pMessage, callback, pInvokeOption, pServiceInvokeHolder);
	}

	template<class T>
	uint32_t CServiceInvoker::sync_invoke(uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, const SInvokeOption* pInvokeOption/* = nullptr */, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		return this->sync_invoke(true, nServiceID, pMessage, pResponseMessage, pInvokeOption, pServiceInvokeHolder);
	}

	template<class T>
	void CServiceInvoker::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, const SInvokeOption* pInvokeOption/* = nullptr */, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAst(pServiceSelector != nullptr);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);

		this->async_invoke(pServiceSelector->isCheckHealth(), nServiceID, pMessage, callback, pInvokeOption, pServiceInvokeHolder);
	}

	template<class T>
	void CServiceInvoker::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, CFuture<T>& sFuture, const SInvokeOption* pInvokeOption/* = nullptr */, CServiceInvokeHolder* pServiceInvokeHolder/* = nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAst(pServiceSelector != nullptr);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);

		this->async_invoke(pServiceSelector->isCheckHealth(), nServiceID, pMessage, sFuture, pInvokeOption, pServiceInvokeHolder);
	}

	template<class T>
	uint32_t CServiceInvoker::sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, std::shared_ptr<T>& pResponseMessage, const SInvokeOption* pInvokeOption/* = nullptr */, CServiceInvokeHolder* pServiceInvokeHolder /*= nullptr*/)
	{
		CServiceSelector* pServiceSelector = this->m_pServiceBase->getServiceSelector(nServiceSelectorType);
		DebugAstEx(pServiceSelector != nullptr, eRRT_ERROR);

		uint32_t nServiceID = pServiceSelector->select(szServiceType, nServiceSelectorType, nServiceSelectorContext);

		return this->sync_invoke(pServiceSelector->isCheckHealth(), nServiceID, pMessage, pResponseMessage, pInvokeOption, pServiceInvokeHolder);
	}
}