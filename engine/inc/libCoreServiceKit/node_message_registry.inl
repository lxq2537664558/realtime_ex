namespace core
{
	template<class T>
	std::map<uint16_t, typename CNodeMessageRegistry<T>::funMessageHandler> CNodeMessageRegistry<T>::s_mapMessageHandler;

	template<class T>
	std::map<uint16_t, typename CNodeMessageRegistry<T>::funForwardHandler> CNodeMessageRegistry<T>::s_mapForwardHandler;

	template<class T>
	void CNodeMessageRegistry<T>::registerMessageHandler(uint16_t nMessageID, bool(T::*handler)(uint16_t, CMessage))
	{
		DebugAst(handler != nullptr);

		s_mapMessageHandler[nMessageID] = handler;
	}

	template<class T>
	void CNodeMessageRegistry<T>::registerForwardHandler(uint16_t nMessageID, bool(T::*handler)(SClientSessionInfo, CMessage))
	{
		DebugAst(handler != nullptr);

		s_mapForwardHandler[nMessageID] = handler;
	}

	template<class T>
	bool CNodeMessageRegistry<T>::dispatch(T* pObject, uint16_t nFrom, CMessage& pMessage)
	{
		auto iter = s_mapMessageHandler.find(pMessage->nMessageID);
		if (iter == s_mapMessageHandler.end())
			return false;

		funMessageHandler handler = iter->second;

		return (pObject->*handler)(nFrom, pMessage);
	}

	template<class T>
	bool CNodeMessageRegistry<T>::forward(T* pObject, SClientSessionInfo& sSession, CMessage& pMessage)
	{
		auto iter = s_mapForwardHandler.find(pMessage->nMessageID);
		if (iter == s_mapForwardHandler.end())
			return false;

		funForwardHandler handler = iter->second;

		return (pObject->*handler)(sSession, pMessage);
	}
}