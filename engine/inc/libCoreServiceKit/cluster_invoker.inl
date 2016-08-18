namespace core
{
	template<class T>
	bool CClusterInvoker::invoke_r(uint16_t nServiceID, const message_header* pData, CFuture<std::shared_ptr<T>>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		std::shared_ptr<CPromise<std::shared_ptr<T>>> pPromise = std::make_shared<CPromise<std::shared_ptr<T>>>();

		auto callback = [pPromise](CMessage pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::static_pointer_cast<T>(pMessage), nErrorCode);
		};

		if (!this->invokeImpl(nServiceID, pData, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	bool CClusterInvoker::invoke_r(const std::string& szServiceName, const message_header* pData, CFuture<std::shared_ptr<T>>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		uint16_t nServiceID = CCoreServiceApp::Inst()->getServiceID(szServiceName);
		if (nServiceID == 0)
			return false;

		return invoke_r(nServiceID, pData, sFuture);
	}

	template<class T>
	bool CClusterInvoker::invoke_r(uint16_t nServiceID, const message_header* pData, const std::function<void(std::shared_ptr<T>, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](CMessage pMessage, uint32_t nErrorCode)->void
		{
			callback(std::static_pointer_cast<T>(pMessage), nErrorCode);
		};

		return this->invokeImpl(nServiceID, pData, callback_);
	}

	template<class T>
	bool CClusterInvoker::invoke_r(const std::string& szServiceName, const message_header* pData, const std::function<void(std::shared_ptr<T>, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		uint16_t nServiceID = CCoreServiceApp::Inst()->getServiceID(szServiceName);
		if (nServiceID == 0)
			return false;

		return invoke_r(nServiceID, pData, callback);
	}
}