namespace core
{
	template<class T>
	CServiceMessageRegistry<T>::CServiceMessageRegistry(uint16_t nServiceID)
		: m_nServiceID(nServiceID)
	{

	}

	template<class T>
	void CServiceMessageRegistry<T>::registerMessageHandler(const std::string& szMessageName, bool(T::*handler)(SServiceSessionInfo, google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapMessageHandler[nMessageID] = handler;
	}

	template<class T>
	void CServiceMessageRegistry<T>::registerForwardHandler(const std::string& szMessageName, bool(T::*handler)(SClientSessionInfo, google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapForwardHandler[nMessageID] = handler;
	}

	template<class T>
	uint16_t CServiceMessageRegistry<T>::getServiceID() const
	{
		return this->m_nServiceID;
	}

	template<class T>
	bool CServiceMessageRegistry<T>::dispatch(T* pObject, SServiceSessionInfo& sServiceSessionInfo, google::protobuf::Message* pMessage)
	{
		auto iter = m_mapMessageHandler.find(pMessage.getMessageID());
		if (iter == m_mapMessageHandler.end())
			return false;

		funMessageHandler handler = iter->second;

		return (pObject->*handler)(sServiceSessionInfo, pMessage);
	}

	template<class T>
	bool CServiceMessageRegistry<T>::forward(T* pObject, SClientSessionInfo& sClientSessionInfo, google::protobuf::Message* pMessage)
	{
		auto iter = m_mapForwardHandler.find(pMessage.getMessageID());
		if (iter == m_mapForwardHandler.end())
			return false;

		funForwardHandler handler = iter->second;

		return (pObject->*handler)(sClientSessionInfo, pMessage);
	}
}