namespace core
{
	template<class T>
	bool CBaseActor::invoke_r(uint64_t nID, const message_header* pData, CFuture<std::shared_ptr<T>>& sFuture)
	{
		DebugAstEx(pData != nullptr, false);

		std::shared_ptr<CPromise<std::shared_ptr<T>>> pPromise = std::make_shared<CPromise<std::shared_ptr<T>>>();

		auto callback = [pPromise](CMessage pMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::static_pointer_cast<T>(pMessage), nErrorCode);
		};

		if (!this->invokeImpl(nID, pData, 0, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	uint32_t CBaseActor::invoke(uint64_t nID, const message_header* pData, std::shared_ptr<T>& pResultMessage)
	{
		if (!this->invokeImpl(nID, pData, coroutine::getCurrentID(), nullptr))
			return eRRT_ERROR;

		coroutine::yield();

		CMessage* pMessage = reinterpret_cast<CMessage*>(coroutine::recvMessage(coroutine::getCurrentID()));
		uint32_t nRet = (uint32_t)reinterpret_cast<uint64_t>(coroutine::recvMessage(coroutine::getCurrentID()));

		pResultMessage = std::static_pointer_cast<T>(*pMessage);
		SAFE_DELETE(pMessage);

		return nRet;
	}

	template<class T>
	bool CBaseActor::invoke_r(uint64_t nID, const message_header* pData, const std::function<void(std::shared_ptr<T>, uint32_t)>& callback)
	{
		DebugAstEx(pData != nullptr && callback != nullptr, false);

		auto callback_ = [callback](CMessage pMessage, uint32_t nErrorCode)->void
		{
			callback(std::static_pointer_cast<T>(pMessage), nErrorCode);
		};

		if (!this->invokeImpl(nID, pData, 0, callback_))
			return false;

		return true;
	}
}