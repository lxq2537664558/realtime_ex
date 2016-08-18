namespace core
{
	template<class T>
	void CActorDispatch<T>::registerMessageHandler(uint16_t nMessageID, void(T::*handler)(uint64_t, CMessage), bool bAsync)
	{
		DebugAst(handler != nullptr);

		SMessageHandlerInfo sMessageHandlerInfo;
		sMessageHandlerInfo.handler = handler;
		sMessageHandlerInfo.bAsync = bAsync;

		getMessageHandlerInfo()[nMessageID] = sMessageHandlerInfo;
	}

	template<class T>
	void CActorDispatch<T>::registerForwardHandler(uint16_t nMessageID, void(T::*handler)(SClientSessionInfo, CMessage), bool bAsync)
	{
		DebugAst(handler != nullptr);

		SForwardHandlerInfo sForwardHandlerInfo;
		sForwardHandlerInfo.handler = handler;
		sForwardHandlerInfo.bAsync = bAsync;

		getForwardHandlerInfo()[nMessageID] = sForwardHandlerInfo;
	}

	template<class T>
	void CActorDispatch<T>::dispatch(T* pObject, uint64_t nFrom, uint8_t nMessageType, CMessage& pMessage)
	{
		auto iter = getMessageHandlerInfo().find(pMessage->nMessageID);
		if (iter == getMessageHandlerInfo().end())
			throw 1;

		SMessageHandlerInfo& sMessageHandlerInfo = iter->second;

		if (!sMessageHandlerInfo.bAsync)
		{
			uint64_t nCoroutineID = coroutine::create([&](uint64_t){ (pObject->*sMessageHandlerInfo.handler)(nFrom, pMessage); });
			coroutine::resume(nCoroutineID, 0);
		}
		else
		{
			(pObject->*sMessageHandlerInfo.handler)(nFrom, pMessage);
		}
	}

	template<class T>
	void CActorDispatch<T>::forward(SClientSessionInfo& sSession, uint8_t nMessageType, CMessage& pMessage)
	{
		auto iter = getForwardHandlerInfo().find(pMessage->nMessageID);
		if (iter == getForwardHandlerInfo().end())
			throw 1;

		SForwardHandlerInfo& sForwardHandlerInfo = iter->second;

		if (!sForwardHandlerInfo.bAsync)
		{
			uint64_t nCoroutineID = coroutine::create([&](uint64_t){ (pObject->*sForwardHandlerInfo.handler)(sSession, pMessage); });
			coroutine::resume(nCoroutineID, 0);
		}
		else
		{
			(pObject->*sForwardHandlerInfo.handler)(sSession, pMessage);
		}
	}
}