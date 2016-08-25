namespace core
{
	template<class T>
	std::map<uint16_t, typename CActorMessageRegistry<T>::funMessageHandler> CActorMessageRegistry<T>::s_mapMessageHandler;

	template<class T>
	std::map<uint16_t, typename CActorMessageRegistry<T>::funForwardHandler> CActorMessageRegistry<T>::s_mapForwardHandler;

	template<class T>
	void CActorMessageRegistry<T>::registerMessageHandler(uint16_t nMessageID, void(T::*handler)(CBaseActor*, uint64_t, CMessage))
	{
		DebugAst(handler != nullptr);

		s_mapMessageHandler[nMessageID] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::registerForwardHandler(uint16_t nMessageID, void(T::*handler)(CBaseActor*, SClientSessionInfo, CMessage))
	{
		DebugAst(handler != nullptr);

		s_mapForwardHandler[nMessageID] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::dispatch(T* pObject, CBaseActor* pBaseActor, uint64_t nFrom, CMessage& pMessage)
	{
		auto iter = s_mapMessageHandler.find(pMessage->nMessageID);
		if (iter == s_mapMessageHandler.end())
			return;

		funMessageHandler handler = iter->second;

		(pObject->*handler)(pBaseActor, nFrom, pMessage);
	}

	template<class T>
	void CActorMessageRegistry<T>::forward(T* pObject, CBaseActor* pBaseActor, SClientSessionInfo& sSession, CMessage& pMessage)
	{
		auto iter = s_mapForwardHandler.find(pMessage->nMessageID);
		if (iter == s_mapForwardHandler.end())
			return;

		funForwardHandler handler = iter->second;

		(pObject->*handler)(pBaseActor, sSession, pMessage);
	}
}