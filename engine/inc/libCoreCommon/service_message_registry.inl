namespace core
{
	template<class T>
	void CServiceMessageRegistry<T>::registerMessageHandler(const std::string& szMessageName, void(T::*handler)(CServiceBase*, SSessionInfo, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapMessageHandler[szMessageName] = handler;
	}

	template<class T>
	void CServiceMessageRegistry<T>::registerForwardHandler(const std::string& szMessageName, void(T::*handler)(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapForwardHandler[szMessageName] = handler;
	}

	template<class T>
	void CServiceMessageRegistry<T>::dispatch(T* pObject, CServiceBase* pServiceBase, SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = m_mapMessageHandler.find(pMessage->GetTypeName());
		if (iter == m_mapMessageHandler.end())
			return;

		funcMessageHandler handler = iter->second;

		(pObject->*handler)(pServiceBase, sSessionInfo, pMessage);
	}

	template<class T>
	void CServiceMessageRegistry<T>::forward(T* pObject, CServiceBase* pServiceBase, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = m_mapForwardHandler.find(pMessage->GetTypeName());
		if (iter == m_mapForwardHandler.end())
			return;

		funcForwardHandler handler = iter->second;

		(pObject->*handler)(pServiceBase, sClientSessionInfo, pMessage);
	}
}