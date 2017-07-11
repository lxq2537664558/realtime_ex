#pragma once
#include "actor_base.h"

#include <map>

namespace core
{
	template<class T>
	class CActorMessageRegistry
	{
	public:
		typedef void(T::*funMessageHandler)(CActorBase*, uint64_t, const google::protobuf::Message*);
		typedef void(T::*funForwardHandler)(CActorBase*, SClientSessionInfo, const google::protobuf::Message*);

	public:
		static inline void	registerMessageHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, SActorSessionInfo, const google::protobuf::Message*));

		static inline void	registerForwardHandler(const std::string& szMessageName, void(T::*handler)(CActorBase*, SClientSessionInfo, const google::protobuf::Message*));

	protected:
		static inline void	dispatch(T* pObject, CActorBase* pActorBase, SActorSessionInfo& sActorSessionInfo, const google::protobuf::Message* pMessage);

		static inline void	forward(T* pObject, CActorBase* pActorBase, SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		static std::map<std::string, funMessageHandler>	s_mapMessageHandler;
		static std::map<std::string, funForwardHandler>	s_mapForwardHandler;
	};
}

#define DEFEND_ACTOR_MESSAGE_FUNCTION(Class) \
		inline void	onDefaultActorMessageHandler(core::CActorBase* pActorBase, SActorSessionInfo& sActorSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CActorMessageRegistry<Class>::dispatch(this, pActorBase, sActorSessionInfo, pMessage);\
		}\
		inline void	onDefaultActorForwardHandler(core::CActorBase* pActorBase, core::SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage)\
		{\
			core::CActorMessageRegistry<Class>::forward(this, pActorBase, sClientSessionInfo, pMessage);\
		}

#define REGISTER_ACTOR_MESSAGE_HANDLER(Class, name, handler)	do { core::CActorMessageRegistry<Class>::registerMessageHandler(name, (core::CActorMessageRegistry<Class>::funMessageHandler)handler); core::CActorBase::registerMessageHandler(id, std::bind(&Class::onDefaultActorMessageHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)
#define REGISTER_ACTOR_FORWARD_HANDLER(Class, name, handler)	do { core::CActorMessageRegistry<Class>::registerForwardHandler(name, (core::CActorMessageRegistry<Class>::funForwardHandler)handler); core::CActorBase::registerForwardHandler(id, std::bind(&Class::onDefaultActorForwardHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)

#include "actor_message_registry.inl"