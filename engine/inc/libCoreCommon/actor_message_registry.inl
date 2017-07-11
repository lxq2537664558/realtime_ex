namespace core
{
	template<class T>
	std::map<std::string, typename CActorMessageRegistry<T>::funMessageHandler> CActorMessageRegistry<T>::s_mapMessageHandler;

	template<class T>
	std::map<std::string, typename CActorMessageRegistry<T>::funForwardHandler> CActorMessageRegistry<T>::s_mapForwardHandler;

	template<class T>
	void CActorMessageRegistry<T>::registerMessageHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, uint64_t, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		s_mapMessageHandler[szMessageName] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::registerForwardHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, SClientSessionInfo, const google::protobuf::Message*))
	{
		DebugAst(handler != nullptr);

		s_mapForwardHandler[szMessageName] = handler;
	}

	template<class T>
	void CActorMessageRegistry<T>::dispatch(T* pObject, CActorBase* pActorBase, SActorSessionInfo& sActorSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = s_mapMessageHandler.find(pMessage->GetTypeName());
		if (iter == s_mapMessageHandler.end())
			return;

		funMessageHandler handler = iter->second;

		(pObject->*handler)(pActorBase, sActorSessionInfo, pMessage);
	}

	template<class T>
	void CActorMessageRegistry<T>::forward(T* pObject, CActorBase* pActorBase, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		DebugAst(pMessage != nullptr);

		auto iter = s_mapForwardHandler.find(pMessage->GetTypeName());
		if (iter == s_mapForwardHandler.end())
			return;

		funForwardHandler handler = iter->second;

		(pObject->*handler)(pActorBase, sClientSessionInfo, pMessage);
	}
}