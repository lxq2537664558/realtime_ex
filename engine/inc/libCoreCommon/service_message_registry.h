#pragma once
#include "protobuf_helper.h"

#include <map>

namespace core
{
	template<class T>
	class CServiceMessageRegistry
	{
	public:
		typedef void(T::*funMessageHandler)(SSessionInfo, const google::protobuf::Message*);
		typedef void(T::*funForwardHandler)(SClientSessionInfo, const google::protobuf::Message*);

	public:
		CServiceMessageRegistry(uint16_t nServiceID);
		~CServiceMessageRegistry();

		inline void	registerMessageHandler(const std::string& szMessageName, void(T::*handler)(SSessionInfo, const google::protobuf::Message*));

		inline void	registerForwardHandler(const std::string& szMessageName, void(T::*handler)(SClientSessionInfo, const google::protobuf::Message*));

		uint16_t	getServiceID() const;

	protected:
		inline void	dispatch(T* pObject, SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		inline void	forward(T* pObject, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		uint16_t									m_nServiceID;
		std::map<std::string, funMessageHandler>	m_mapMessageHandler;
		std::map<std::string, funForwardHandler>	m_mapForwardHandler;
	};
}

#define DEFEND_SERVICE_MESSAGE_FUNCTION(Class) \
		inline void	onDefaultServiceMessageHandler(core::SSessionInfo sSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CServiceMessageRegistry<Class>::dispatch(this, sSessionInfo, pMessage); \
		}\
		inline void	onDefaultServiceForwardHandler(core::SClientSessionInfo sClientSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CServiceMessageRegistry<Class>::forward(this, sClientSessionInfo, pMessage);\
		}

#define REGISTER_SERVICE_MESSAGE_HANDLER(Class, name, handler)	do { core::CServiceMessageRegistry<Class>::registerMessageHandler(name, (core::CServiceMessageRegistry<Class>::funMessageHandler)handler); core::CBaseApp::Inst()->registerMessageHandler(this->getServiceID(), name, std::bind(&Class::onDefaultServiceMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); } while(0)
#define REGISTER_SERVICE_FORWARD_HANDLER(Class, name, handler)	do { core::CServiceMessageRegistry<Class>::registerForwardHandler(name, (core::CServiceMessageRegistry<Class>::funForwardHandler)handler); core::CBaseApp::Inst()->registerForwardHandler(this->getServiceID(), name, std::bind(&Class::onDefaultServiceForwardHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); } while(0)

#include "service_message_registry.inl"