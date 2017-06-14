namespace core
{
	template<class T>
	std::map<uint16_t, typename CServiceMessageRegistry<T>::funMessageHandler> CServiceMessageRegistry<T>::s_mapMessageHandler;

	template<class T>
	std::map<uint16_t, typename CServiceMessageRegistry<T>::funForwardHandler> CServiceMessageRegistry<T>::s_mapForwardHandler;

	template<class T>
	void CServiceMessageRegistry<T>::registerMessageHandler(uint16_t nMessageID, bool(T::*handler)(uint16_t, CMessagePtr<char>))
	{
		DebugAst(handler != nullptr);

		s_mapMessageHandler[nMessageID] = handler;
	}

	template<class T>
	void CServiceMessageRegistry<T>::registerForwardHandler(uint16_t nMessageID, bool(T::*handler)(SClientSessionInfo, CMessagePtr<char>))
	{
		DebugAst(handler != nullptr);

		s_mapForwardHandler[nMessageID] = handler;
	}

	template<class T>
	bool CServiceMessageRegistry<T>::dispatch(T* pObject, uint16_t nFrom, CMessagePtr<char>& pMessage)
	{
		auto iter = s_mapMessageHandler.find(pMessage.getMessageID());
		if (iter == s_mapMessageHandler.end())
			return false;

		funMessageHandler handler = iter->second;

		return (pObject->*handler)(nFrom, pMessage);
	}

	template<class T>
	bool CServiceMessageRegistry<T>::forward(T* pObject, SClientSessionInfo& sSession, CMessagePtr<char>& pMessage)
	{
		auto iter = s_mapForwardHandler.find(pMessage.getMessageID());
		if (iter == s_mapForwardHandler.end())
			return false;

		funForwardHandler handler = iter->second;

		return (pObject->*handler)(sSession, pMessage);
	}
}