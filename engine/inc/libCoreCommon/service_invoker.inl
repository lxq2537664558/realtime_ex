namespace core
{
	template<class T>
	bool CServiceInvoker::async_call(uint16_t nServiceID, const void* pData, CFuture<T>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		auto pPromise = std::make_shared<CPromise<CMessagePtr<T>>>();
		
		auto callback = [pPromise](CMessagePtr<char> pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(CMessagePtr<T>::reinterpret_cast_message(pMessage), nErrorCode);
		};

		if (!this->invoke(nNodeID, pData, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	bool CServiceInvoker::async_call(uint16_t nServiceID, const void* pData, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](CMessagePtr<char> pMessage, uint32_t nErrorCode)->void
		{
			callback(CMessagePtr<T>::reinterpret_cast_message(pMessage), nErrorCode);
		};

		return this->invoke(nNodeID, pData, callback_);
	}
}