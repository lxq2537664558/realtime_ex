namespace core
{
	template<class T>
	bool CServiceInvoker::async_call(uint16_t nServiceID, const void* pData, CFuture<T>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		auto pPromise = std::make_shared<CPromise<T>>();
		
		auto callback = [pPromise](const google::protobuf::Message* pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(pMessage, nErrorCode);
		};

		if (!this->invoke(nServiceID, pData, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	bool CServiceInvoker::async_call(uint16_t nServiceID, const void* pData, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](const google::protobuf::Message* pMessage, uint32_t nErrorCode)->void
		{
			callback(pMessage, nErrorCode);
		};

		return this->invoke(nServiceID, pData, callback_);
	}
}