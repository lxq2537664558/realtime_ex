
template<class T, class M>
void register_service_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const google::protobuf::Message* pMessage)
	{
		(pObject->*handler)(pServiceBase, sSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceMessageHandler(szName, callback);
}

template<class T, class M>
void register_forward_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SClientSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SClientSessionInfo sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		(pObject->*handler)(pServiceBase, sClientSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceForwardHandler(szName, callback);
}

template<class T, class M>
void register_actor_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CActorBase* pActorBase, core::SSessionInfo sSessionInfo, const google::protobuf::Message* pMessage)
	{
		(pObject->*handler)(pActorBase, sSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerActorMessageHandler(szName, callback);
}

template<class T, class M>
void register_actor_forward_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SClientSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CActorBase* pActorBase, core::SClientSessionInfo sClientSessionInfo, const google::protobuf::Message* pMessage)
	{
		(pObject->*handler)(pActorBase, sClientSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerActorForwardHandler(szName, callback);
}