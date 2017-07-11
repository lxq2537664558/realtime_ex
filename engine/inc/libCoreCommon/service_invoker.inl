namespace core
{
	template<class T>
	bool CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		auto pPromise = std::make_shared<CPromise<T>>();
		
		auto callback = [pPromise](const google::protobuf::Message* pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(pMessage, nErrorCode);
		};

		if (!this->invoke(eType, nID, pData, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	bool CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](const google::protobuf::Message* pMessage, uint32_t nErrorCode)->void
		{
			callback(pMessage, nErrorCode);
		};

		return this->invoke(eType, nID, pData, callback_);
	}
}