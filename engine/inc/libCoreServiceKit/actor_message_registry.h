#pragma once
#include "base_actor.h"

#include <map>

namespace core
{
	template<class T>
	class CActorMessageRegistry
	{
	public:
		typedef void(T::*funMessageHandler)(CBaseActor*, uint64_t, CMessage);
		typedef void(T::*funForwardHandler)(CBaseActor*, SClientSessionInfo, CMessage);

	public:
		static inline void	registerMessageHandler(uint16_t nMessageID, void(T::*handler)(CBaseActor*, uint64_t, CMessage));

		static inline void	registerForwardHandler(uint16_t nMessageID, void(T::*handler)(CBaseActor*, SClientSessionInfo, CMessage));

	protected:
		static inline void	dispatch(T* pObject, CBaseActor* pBaseActor, uint64_t nFrom, CMessage& pMessage);

		static inline void	forward(T* pObject, CBaseActor* pBaseActor, SClientSessionInfo& sSession, CMessage& pMessage);

	private:
		static std::map<uint16_t, funMessageHandler>	s_mapMessageHandler;
		static std::map<uint16_t, funForwardHandler>	s_mapForwardHandler;
	};
}

#define DEFEND_ACTOR_MESSAGE_FUNCTION(Class) \
		inline void	onDefaultActorMessageHandler(core::CBaseActor* pBaseActor, uint64_t nFrom, core::CMessage pMessage)\
		{\
			core::CActorMessageRegistry<Class>::dispatch(this, pBaseActor, nFrom, pMessage);\
		}\
		inline void	onDefaultActorForwardHandler(core::CBaseActor* pBaseActor, core::SClientSessionInfo& sClientSessionInfo, core::CMessage pMessage)\
		{\
			core::CActorMessageRegistry<Class>::forward(this, pBaseActor, sClientSessionInfo, pMessage);\
		}

#define REGISTER_ACTOR_MESSAGE_HANDLER(Class, id, handler, _async)	do { core::CActorMessageRegistry<Class>::registerMessageHandler(id, (core::CActorMessageRegistry<Class>::funMessageHandler)handler); core::CBaseActor::registerMessageHandler(id, std::bind(&Class::onDefaultActorMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), _async); } while(0)
#define REGISTER_ACTOR_FORWARD_HANDLER(Class, id, handler, _async)	do { core::CActorMessageRegistry<Class>::registerForwardHandler(id, (core::CActorMessageRegistry<Class>::funForwardHandler)handler); core::CBaseActor::registerForwardHandler(id, std::bind(&Class::onDefaultActorForwardHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), _async); } while(0)

#include "actor_message_registry.inl"