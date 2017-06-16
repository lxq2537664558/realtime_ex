#pragma once
#include "actor_base.h"

#include <map>

namespace core
{
	template<class T>
	class CActorMessageRegistry
	{
	public:
		typedef void(T::*funMessageHandler)(CActorBase*, uint64_t, CMessagePtr<char>);
		typedef void(T::*funForwardHandler)(CActorBase*, SClientSessionInfo, CMessagePtr<char>);

	public:
		static inline void	registerMessageHandler(uint16_t nMessageID, void(T::*handler)(CActorBase*, uint64_t, CMessagePtr<char>));

		static inline void	registerForwardHandler(uint16_t nMessageID, void(T::*handler)(CActorBase*, SClientSessionInfo, CMessagePtr<char>));

	protected:
		static inline void	dispatch(T* pObject, CActorBase* pActorBase, uint64_t nFrom, CMessagePtr<char>& pMessage);

		static inline void	forward(T* pObject, CActorBase* pActorBase, SClientSessionInfo& sSession, CMessagePtr<char>& pMessage);

	private:
		static std::map<uint16_t, funMessageHandler>	s_mapMessageHandler;
		static std::map<uint16_t, funForwardHandler>	s_mapForwardHandler;
	};
}

#define DEFEND_ACTOR_MESSAGE_FUNCTION(Class) \
		inline void	onDefaultActorMessageHandler(core::CActorBase* pActorBase, uint64_t nFrom, core::CMessagePtr<char> pMessage)\
		{\
			core::CActorMessageRegistry<Class>::dispatch(this, pActorBase, nFrom, pMessage);\
		}\
		inline void	onDefaultActorForwardHandler(core::CActorBase* pActorBase, core::SClientSessionInfo& sClientSessionInfo, core::CMessagePtr<char> pMessage)\
		{\
			core::CActorMessageRegistry<Class>::forward(this, pActorBase, sClientSessionInfo, pMessage);\
		}

#define REGISTER_ACTOR_MESSAGE_HANDLER(Class, id, handler, _async)	do { core::CActorMessageRegistry<Class>::registerMessageHandler(id, (core::CActorMessageRegistry<Class>::funMessageHandler)handler); core::CActorBase::registerMessageHandler(id, std::bind(&Class::onDefaultActorMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), _async); } while(0)
#define REGISTER_ACTOR_FORWARD_HANDLER(Class, id, handler, _async)	do { core::CActorMessageRegistry<Class>::registerForwardHandler(id, (core::CActorMessageRegistry<Class>::funForwardHandler)handler); core::CActorBase::registerForwardHandler(id, std::bind(&Class::onDefaultActorForwardHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), _async); } while(0)

#include "actor_message_registry.inl"