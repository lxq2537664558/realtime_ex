#pragma once

#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "service_base.h"

#include <map>

namespace core
{
	template<class T>
	class CServiceMessageRegistry
	{
	public:
		typedef void(T::*funMessageHandler)(CServiceBase*, SSessionInfo, const google::protobuf::Message*);
		typedef void(T::*funForwardHandler)(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*);

	public:
		CServiceMessageRegistry() { }
		virtual ~CServiceMessageRegistry() { }

		inline void	registerMessageHandler(const std::string& szMessageName, void(T::*handler)(CServiceBase*, SSessionInfo, const google::protobuf::Message*));

		inline void	registerForwardHandler(const std::string& szMessageName, void(T::*handler)(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*));

		uint32_t	getServiceID() const;

	protected:
		inline void	dispatch(T* pObject, CServiceBase* pServiceBase, SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		inline void	forward(T* pObject, CServiceBase* pServiceBase, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		std::map<std::string, funMessageHandler>	m_mapMessageHandler;
		std::map<std::string, funForwardHandler>	m_mapForwardHandler;
	};
}

#define DEFEND_SERVICE_MESSAGE_FUNCTION(Class) \
		inline void	onDefaultServiceMessageHandler(CServiceBase* pServiceBase, core::SSessionInfo sSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CServiceMessageRegistry<Class>::dispatch(this, pServiceBase, sSessionInfo, pMessage); \
		}\
		inline void	onDefaultServiceForwardHandler(CServiceBase* pServiceBase, core::SClientSessionInfo sClientSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CServiceMessageRegistry<Class>::forward(this, pServiceBase, sClientSessionInfo, pMessage);\
		}

#define REGISTER_SERVICE_MESSAGE_HANDLER(pServiceBase, Class, name, handler)	do { core::CServiceMessageRegistry<Class>::registerMessageHandler(name, (core::CServiceMessageRegistry<Class>::funMessageHandler)handler); pServiceBase->registerServiceMessageHandler(name, std::bind(&Class::onDefaultServiceMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); } while(0)
#define REGISTER_SERVICE_FORWARD_HANDLER(pServiceBase, Class, name, handler)	do { core::CServiceMessageRegistry<Class>::registerForwardHandler(name, (core::CServiceMessageRegistry<Class>::funForwardHandler)handler); pServiceBase->registerServiceForwardHandler(name, std::bind(&Class::onDefaultServiceForwardHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); } while(0)

#include "service_message_registry.inl"