namespace core
{
	template<class T>
	bool CActorBase::async_call(uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](const google::protobuf::Message* pMessage, uint32_t nErrorCode)->void
		{
			callback(pMessage, nErrorCode);
		};

		if (!this->invoke(nID, pData, 0, callback_))
			return false;

		return true;
	}

	template<class T>
	bool CActorBase::async_call(uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		auto pPromise = std::make_shared<CPromise<T>>();

		auto callback = [pPromise](const google::protobuf::Message* pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(pMessage, nErrorCode);
		};

		if (!this->invoke(nID, pData, 0, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	uint32_t CActorBase::sync_call(uint64_t nID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage)
	{
		if (!this->invoke(nID, pMessage, coroutine::getCurrentID(), nullptr))
			return eRRT_ERROR;

		coroutine::yield();

		uint64_t nMessage = 0;
		uint64_t nRet = 0;
		DebugAstEx(coroutine::getLocalData(coroutine::getCurrentID(), "message", nMessage), eRRT_ERROR);
		DebugAstEx(coroutine::getLocalData(coroutine::getCurrentID(), "result", nRet), eRRT_ERROR);

		auto pMessage = *reinterpret_cast<std::shared_ptr<google::protobuf::Message>*>(nMessage);

		pResponseMessage = pMessage;

		return (uint32_t)nRet;
	}
}