namespace core
{
	template<class T>
	bool CActorBase::async_call(uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](CMessagePtr<char> pMessage, uint32_t nErrorCode)->void
		{
			callback(CMessagePtr<T>::reinterpret_cast_message(pMessage), nErrorCode);
		};

		if (!this->invokeImpl(nID, pData, 0, callback_))
			return false;

		return true;
	}

	template<class T>
	bool CActorBase::async_call(uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		auto pPromise = std::make_shared<CPromise<CMessagePtr<T>>>();

		auto callback = [pPromise](CMessagePtr<char> pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(CMessagePtr<T>::reinterpret_cast_message(pMessage), nErrorCode);
		};

		if (!this->invokeImpl(nID, pData, 0, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	uint32_t CActorBase::sync_call(uint64_t nID, const google::protobuf::Message* pMessage, std::shared_ptr<R>& pResponseMessage)
	{
		if (!this->invokeImpl(nID, pMessage, coroutine::getCurrentID(), nullptr))
			return eRRT_ERROR;

		coroutine::yield();

		CMessage* pMessage = reinterpret_cast<CMessage*>(coroutine::recvMessage(coroutine::getCurrentID()));
		uint32_t nRet = (uint32_t)reinterpret_cast<uint64_t>(coroutine::recvMessage(coroutine::getCurrentID()));

		pResponseMessage = CMessagePtr<T>::reinterpret_cast_message(*pMessage);
		SAFE_DELETE(pMessage);

		return nRet;
	}
}