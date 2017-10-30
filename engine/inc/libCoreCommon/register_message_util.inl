#include "google/protobuf/message.h"
#include "native_message.h"

template <uint32_t>
inline std::string getMessageName(const void* msg)
{
	return "";
}

template <>
inline std::string getMessageName<core::eMST_Protobuf>(const void* msg)
{
	return reinterpret_cast<const google::protobuf::Message*>(msg)->GetTypeName();
}

template <>
inline std::string getMessageName<core::eMST_Native>(const void* msg)
{
	return reinterpret_cast<const core::native_message_header*>(msg)->getMessageName();
}

template<class T, class M>
void register_service_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SSessionInfo, const M*))
{
	M _msg;
	std::string szName;

	if (std::is_base_of<google::protobuf::Message, M>())
		szName = getMessageName<core::eMST_Protobuf>(&_msg);
	else if (std::is_base_of<core::native_message_header, M>())
		szName = getMessageName<core::eMST_Native>(&_msg);
	
	DebugAst(!szName.empty());

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pServiceBase, sSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceMessageHandler(szName, callback);
}

template<class T, class M>
void register_forward_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SClientSessionInfo, const M*))
{
	M _msg;
	std::string szName;

	if (std::is_base_of<google::protobuf::Message, M>())
		szName = getMessageName<core::eMST_Protobuf>(&_msg);
	else if (std::is_base_of<core::native_message_header, M>())
		szName = getMessageName<core::eMST_Native>(&_msg);

	DebugAst(!szName.empty());

	auto callback = [pObject, handler](core::CServiceBase* pServiceBase, core::SClientSessionInfo sClientSessionInfo, const void* pMessage)
	{
		(pObject->*handler)(pServiceBase, sClientSessionInfo, static_cast<const M*>(pMessage));
	};

	pServiceBase->registerServiceForwardHandler(szName, callback);
}
