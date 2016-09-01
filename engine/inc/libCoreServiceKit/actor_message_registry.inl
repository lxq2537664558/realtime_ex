namespace core
{
	template<class T>
	std::map<uint16_t, typename CActorMessageRegistry<T>::funMessageHandler> CActorMessageRegistry<T>::s_mapMessageHandler;

	template<class T>
	std::map<uint16_t, typename CActorMessageRegistry<T>::funForwardHandler> CActorMessageRegistry<T>::s_mapForwardHandler;

	template<class T>
	void CActorMessageRegistry<T>::registerMessageHandler(uint16_t nMessageID, void(T::*handler)(CBaseActor*, uint64_t, CMessagePtr<char>))
	{
		DebugAst(handler != nullptr);

		s_mapMessageHandler[nMessageID] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::registerForwardHandler(uint16_t nMessageID, void(T::*handler)(CBaseActor*, SClientSessionInfo, CMessagePtr<char>))
	{
		DebugAst(handler != nullptr);

		s_mapForwardHandler[nMessageID] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::dispatch(T* pObject, CBaseActor* pBaseActor, uint64_t nFrom, CMessagePtr<char>& pMessage)
	{
		auto iter = s_mapMessageHandler.find(pMessage.getMessageID());
		if (iter == s_mapMessageHandler.end())
			return;

		funMessageHandler handler = iter->second;

		(pObject->*handler)(pBaseActor, nFrom, pMessage);
	}

	template<class T>
	void CActorMessageRegistry<T>::forward(T* pObject, CBaseActor* pBaseActor, SClientSessionInfo& sSession, CMessagePtr<char>& pMessage)
	{
		auto iter = s_mapForwardHandler.find(pMessage.getMessageID());
		if (iter == s_mapForwardHandler.end())
			return;

		funForwardHandler handler = iter->second;

		(pObject->*handler)(pBaseActor, sSession, pMessage);
	}
}