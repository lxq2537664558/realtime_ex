namespace core
{
	template<class T>
	bool CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAstEx(pMessage != nullptr, false);

		auto pPromise = std::make_shared<CPromise<T>>();
		
		auto callback = [pPromise](const google::protobuf::Message* pResponseMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(const_cast<T*>(reinterpret_cast<const T*>(pResponseMessage)), nErrorCode);
		};

		if (!this->invoke(eType, nID, pMessage, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	bool CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr, false);

		auto callback_ = [callback](const google::protobuf::Message* pResponseMessage, uint32_t nErrorCode)->void
		{
			callback(reinterpret_cast<const T*>(pResponseMessage), nErrorCode);
		};

		return this->invoke(eType, nID, pMessage, callback_);
	}
}