#pragma once
#include "protobuf_helper.h"

#include <map>

namespace core
{
	template<class T>
	class CServiceMessageRegistry
	{
	public:
		typedef bool(T::*funMessageHandler)(SServiceSessionInfo, google::protobuf::Message*);
		typedef bool(T::*funForwardHandler)(SClientSessionInfo, google::protobuf::Message*);

	public:
		CServiceMessageRegistry(uint16_t nServiceID);
		~CServiceMessageRegistry();

		inline void	registerMessageHandler(const std::string& szMessageName, bool(T::*handler)(SServiceSessionInfo, google::protobuf::Message*));

		inline void	registerForwardHandler(const std::string& szMessageName, bool(T::*handler)(SClientSessionInfo, google::protobuf::Message*));

		uint16_t	getServiceID() const;

	protected:
		inline bool	dispatch(T* pObject, SServiceSessionInfo& sServiceSessionInfo, google::protobuf::Message* pMessage);

		inline bool	forward(T* pObject, SClientSessionInfo& sClientSessionInfo, google::protobuf::Message* pMessage);

	private:
		uint16_t									m_nServiceID;
		std::map<std::string, funMessageHandler>	m_mapMessageHandler;
		std::map<std::string, funForwardHandler>	m_mapForwardHandler;
	};
}

#define DEFEND_SERVICE_MESSAGE_FUNCTION(Class) \
		inline bool	onDefaultServiceMessageHandler(SServiceSessionInfo sServiceSessionInfo, google::protobuf::Message* pMessage)\
		{\
			return core::CServiceMessageRegistry<Class>::dispatch(this, sServiceSessionInfo, pMessage);\
		}\
		inline bool	onDefaultServiceForwardHandler(core::SClientSessionInfo& sClientSessionInfo, google::protobuf::Message* pMessage)\
		{\
			return core::CServiceMessageRegistry<Class>::forward(this, sClientSessionInfo, pMessage);\
		}

#define REGISTER_SERVICE_MESSAGE_HANDLER(Class, id, handler)	do { core::CServiceMessageRegistry<Class>::registerMessageHandler(id, (core::CServiceMessageRegistry<Class>::funMessageHandler)handler); core::CCoreServiceApp::Inst()->registerMessageHandler(this->getServiceID(), id, std::bind(&Class::onDefaultServiceMessageHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)
#define REGISTER_SERVICE_FORWARD_HANDLER(Class, id, handler)	do { core::CServiceMessageRegistry<Class>::registerForwardHandler(id, (core::CServiceMessageRegistry<Class>::funForwardHandler)handler); core::CCoreServiceApp::Inst()->registerForwardHandler(this->getServiceID(), id, std::bind(&Class::onDefaultServiceForwardHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)

#include "service_message_registry.inl"