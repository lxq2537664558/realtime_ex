#pragma once
#include "actor_base.h"

#include <map>

namespace core
{
	template<class T>
	class CActorMessageRegistry
	{
	public:
		typedef void(T::*funMessageHandler)(CActorBase*, SSessionInfo, const google::protobuf::Message*);
		typedef void(T::*funForwardHandler)(CActorBase*, SClientSessionInfo, const google::protobuf::Message*);

	public:
		inline void	registerMessageHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, SSessionInfo, const google::protobuf::Message*));

		inline void	registerForwardHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, SClientSessionInfo, const google::protobuf::Message*));

	protected:
		inline void	dispatch(T* pObject, CActorBase* pActorBase, SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		inline void	forward(T* pObject, CActorBase* pActorBase, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		std::map<std::string, funMessageHandler>	m_mapMessageHandler;
		std::map<std::string, funForwardHandler>	m_mapForwardHandler;
	};
}

#define DEFEND_ACTOR_MESSAGE_FUNCTION(Class) \
		inline void	onDefaultActorMessageHandler(core::CActorBase* pActorBase, core::SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CActorMessageRegistry<Class>::dispatch(this, pActorBase, sSessionInfo, pMessage); \
		}\
		inline void	onDefaultActorForwardHandler(core::CActorBase* pActorBase, core::SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CActorMessageRegistry<Class>::forward(this, pActorBase, sClientSessionInfo, pMessage);\
		}

#define REGISTER_ACTOR_MESSAGE_HANDLER(pServiceBase, Class, name, handler)	do { core::CActorMessageRegistry<Class>::registerMessageHandler(name, (core::CActorMessageRegistry<Class>::funMessageHandler)handler); pServiceBase->registerActorMessageHandler(name, std::bind(&Class::onDefaultActorMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); } while(0)
#define REGISTER_ACTOR_FORWARD_HANDLER(pServiceBase, Class, name, handler)	do { core::CActorMessageRegistry<Class>::registerForwardHandler(name, (core::CActorMessageRegistry<Class>::funForwardHandler)handler); pServiceBase->registerActorForwardHandler(name, std::bind(&Class::onDefaultActorForwardHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); } while(0)

#include "actor_message_registry.inl"