namespace core
{
	template<class T>
	bool CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAstEx(pMessage != nullptr, false);

		auto pPromise = std::make_shared<CPromise<T>>();
		
		auto callback = [pPromise](std::shared_ptr<google::protobuf::Message>& pResponseMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::dynamic_pointer_cast<T>(pResponseMessage), nErrorCode);
		};

		if (!this->invoke(eType, nID, pMessage, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	bool CServiceInvoker::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr, false);

		auto callback_ = [callback](std::shared_ptr<google::protobuf::Message>& pResponseMessage, uint32_t nErrorCode)->void
		{
			callback(dynamic_cast<T*>(pResponseMessage.get()), nErrorCode);
		};

		return this->invoke(eType, nID, pMessage, callback_);
	}
}