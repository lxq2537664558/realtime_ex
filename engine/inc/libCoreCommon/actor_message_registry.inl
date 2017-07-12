namespace core
{
	template<class T>
	void CActorMessageRegistry<T>::registerMessageHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, SSessionInfo, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapMessageHandler[szMessageName] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::registerForwardHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, SClientSessionInfo, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		m_mapForwardHandler[szMessageName] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::dispatch(T* pObject, CActorBase* pActorBase, SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = m_mapMessageHandler.find(pMessage->GetTypeName());
		if (iter == m_mapMessageHandler.end())
			return;

		funMessageHandler handler = iter->second;

		(pObject->*handler)(pActorBase, sSessionInfo, pMessage);
	}

	template<class T>
	void CActorMessageRegistry<T>::forward(T* pObject, CActorBase* pActorBase, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = m_mapForwardHandler.find(pMessage->GetTypeName());
		if (iter == m_mapForwardHandler.end())
			return;

		funForwardHandler handler = iter->second;

		(pObject->*handler)(pActorBase, sClientSessionInfo, pMessage);
	}
}