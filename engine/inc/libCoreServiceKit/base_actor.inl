namespace core
{
	template<class T>
	bool CBaseActor::invoke_r(uint64_t nID, const void* pData, CFuture<CMessagePtr<T>>& sFuture)
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
	uint32_t CBaseActor::invoke(uint64_t nID, const void* pData, CMessagePtr<T>& pResultMessage)
	{
		if (!this->invokeImpl(nID, pData, coroutine::getCurrentID(), nullptr))
			return eRRT_ERROR;

		coroutine::yield();

		CMessage* pMessage = reinterpret_cast<CMessage*>(coroutine::recvMessage(coroutine::getCurrentID()));
		uint32_t nRet = (uint32_t)reinterpret_cast<uint64_t>(coroutine::recvMessage(coroutine::getCurrentID()));

		pResultMessage = CMessagePtr<T>::reinterpret_cast_message(*pMessage);
		SAFE_DELETE(pMessage);

		return nRet;
	}

	template<class T>
	bool CBaseActor::invoke_r(uint64_t nID, const void* pData, const std::function<void(CMessagePtr<T>, uint32_t)>& callback)
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
}