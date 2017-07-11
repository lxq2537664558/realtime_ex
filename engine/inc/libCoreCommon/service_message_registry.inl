namespace core
{
	template<class T>
	CServiceMessageRegistry<T>::CServiceMessageRegistry(uint16_t nServiceID)
		: m_nServiceID(nServiceID)
	{

	}

	template<class T>
	void CServiceMessageRegistry<T>::registerMessageHandler(const std::string& szMessageName, void(T::*handler)(SSessionInfo, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapMessageHandler[szMessageName] = handler;
	}

	template<class T>
	void CServiceMessageRegistry<T>::registerForwardHandler(const std::string& szMessageName, void(T::*handler)(SClientSessionInfo, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapForwardHandler[szMessageName] = handler;
	}

	template<class T>
	uint16_t CServiceMessageRegistry<T>::getServiceID() const
	{
		return this->m_nServiceID;
	}

	template<class T>
	void CServiceMessageRegistry<T>::dispatch(T* pObject, SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = m_mapMessageHandler.find(pMessage->GetTypeName());
		if (iter == m_mapMessageHandler.end())
			return;

		funMessageHandler handler = iter->second;

		(pObject->*handler)(sSessionInfo, pMessage);
	}

	template<class T>
	void CServiceMessageRegistry<T>::forward(T* pObject, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = m_mapForwardHandler.find(pMessage->GetTypeName());
		if (iter == m_mapForwardHandler.end())
			return;

		funForwardHandler handler = iter->second;

		(pObject->*handler)(sClientSessionInfo, pMessage);
	}
}