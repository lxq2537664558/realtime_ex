namespace core
{
	template<class T>
	bool CClusterInvoker::invoke_r(uint16_t nNodeID, const void* pData, CFuture<CMessagePtr<T>>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		auto pPromise = std::make_shared<CPromise<CMessagePtr<T>>>();
		
		auto callback = [pPromise](CMessagePtr<char> pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(CMessagePtr<T>::reinterpret_cast_message(pMessage), nErrorCode);
		};

		if (!this->invokeImpl(nNodeID, pData, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	bool CClusterInvoker::invoke_r(uint16_t nNodeID, const void* pData, const std::function<void(CMessagePtr<T>, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](CMessagePtr<char> pMessage, uint32_t nErrorCode)->void
		{
			callback(CMessagePtr<T>::reinterpret_cast_message(pMessage), nErrorCode);
		};

		return this->invokeImpl(nNodeID, pData, callback_);
	}
}