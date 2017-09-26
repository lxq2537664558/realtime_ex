
template<class T, class M>
void register_pb_service_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pServiceBase, sSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceMessageHandler(szName, callback);
}

template<class T, class M>
void register_pb_forward_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SClientSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SClientSessionInfo sClientSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pServiceBase, sClientSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceForwardHandler(szName, callback);
}

template<class T, class M>
void register_pb_actor_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CActorBase* pActorBase, core::SSessionInfo sSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pActorBase, sSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerActorMessageHandler(szName, callback);
}

template<class T, class M>
void register_pb_actor_forward_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SClientSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.GetTypeName();

	auto callback = [pObject, handler](core::CActorBase* pActorBase, core::SClientSessionInfo sClientSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pActorBase, sClientSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerActorForwardHandler(szName, callback);
}





template<class T, class M>
void register_native_service_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.getMessageName();

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pServiceBase, sSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceMessageHandler(szName, callback);
}

template<class T, class M>
void register_native_forward_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SClientSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.getMessageName();

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SClientSessionInfo sClientSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pServiceBase, sClientSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceForwardHandler(szName, callback);
}

template<class T, class M>
void register_native_actor_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.getMessageName();

	auto callback = [pObject, handler](core::CActorBase* pActorBase, core::SSessionInfo sSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pActorBase, sSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerActorMessageHandler(szName, callback);
}

template<class T, class M>
void register_native_actor_forward_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SClientSessionInfo, const M*))
{
	M _msg;
	std::string szName = _msg.getMessageName();

	auto callback = [pObject, handler](core::CActorBase* pActorBase, core::SClientSessionInfo sClientSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pActorBase, sClientSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerActorForwardHandler(szName, callback);
}