namespace core
{
	template<class T>
	bool CActorBase::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback)
	{
		DebugAstEx(pMessage != nullptr && callback != nullptr, false);

		auto callback_ = [callback](std::shared_ptr<google::protobuf::Message>& pResponseMessage, uint32_t nErrorCode)->void
		{
			callback(pResponseMessage.get(), nErrorCode);
		};

		if (!this->invoke(eType, nID, pMessage, 0, callback_))
			return false;

		return true;
	}

	template<class T>
	bool CActorBase::async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture)
	{
		DebugAstEx(pMessage != nullptr, false);

		auto pPromise = std::make_shared<CPromise<T>>();

		auto callback = [pPromise](std::shared_ptr<google::protobuf::Message>& pResponseMessage, uint32_t nErrorCode)->void
		{
			pPromise->setValue(std::dynamic_pointer_cast<T>(pResponseMessage), nErrorCode);
		};

		if (!this->invoke(eType, nID, pMessage, 0, callback))
			return false;

		sFuture = pPromise->getFuture();

		return true;
	}

	template<class T>
	uint32_t CActorBase::sync_call(uint16_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage)
	{
		if (!this->invoke(eMTT_Service, nServiceID, pMessage, coroutine::getCurrentID(), nullptr))
			return eRRT_ERROR;

		coroutine::yield();

		uint64_t nResponse = 0;
		
		DebugAstEx(coroutine::getLocalData(coroutine::getCurrentID(), "response", nResponse), eRRT_ERROR);
		coroutine::delLocalData(coroutine::getCurrentID(), "response");

		SSyncCallResultInfo* pSyncCallResultInfo = reinterpret_cast<SSyncCallResultInfo*>(nResponse);
		pResponseMessage = std::dynamic_pointer_cast<T>(pSyncCallResultInfo->pMessage);

		uint8_t nResult = pSyncCallResultInfo->nResult;
		
		SAFE_DELETE(pSyncCallResultInfo);

		return nResult;
	}
}