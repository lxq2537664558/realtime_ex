namespace core
{
	template<class T>
	void CServiceInvokeHolder::async_invoke(uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, const SInvokeOption* pInvokeOption/* = nullptr*/)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(nServiceID, pMessage, sFuture, pInvokeOption, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_invoke(uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, const SInvokeOption* pInvokeOption/* = nullptr*/)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(nServiceID, pMessage, callback, pInvokeOption, this);
	}

	template<class T>
	uint32_t CServiceInvokeHolder::sync_invoke(uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, const SInvokeOption* pInvokeOption/* = nullptr*/)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, eRRT_ERROR);

		return this->m_pServiceBase->getServiceInvoker()->sync_invoke(nServiceID, pMessage, pResponseMessage, pInvokeOption, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, const SInvokeOption* pInvokeOption/* = nullptr*/)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage, callback, pInvokeOption, this);
	}

	template<class T>
	void CServiceInvokeHolder::async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, CFuture<T>& sFuture, const SInvokeOption* pInvokeOption/* = nullptr*/)
	{
		DebugAst(this->m_pServiceBase != nullptr);

		this->m_pServiceBase->getServiceInvoker()->async_invoke(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage, sFuture, pInvokeOption, this);
	}

	template<class T>
	uint32_t CServiceInvokeHolder::sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, std::shared_ptr<T>& pResponseMessage, const SInvokeOption* pInvokeOption/* = nullptr*/)
	{
		DebugAstEx(this->m_pServiceBase != nullptr, eRRT_ERROR);

		return this->m_pServiceBase->getServiceInvoker()->sync_invoke(szServiceType, nServiceSelectorType, nServiceSelectorContext, pMessage, pResponseMessage, pInvokeOption, this);
	}
}