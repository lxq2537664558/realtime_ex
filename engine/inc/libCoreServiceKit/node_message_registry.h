#pragma once
#include "base_actor.h"

#include <map>

namespace core
{
	template<class T>
	class CNodeMessageRegistry
	{
	public:
		typedef bool(T::*funMessageHandler)(uint16_t, CMessage);
		typedef bool(T::*funForwardHandler)(SClientSessionInfo, CMessage);

	public:
		static inline void	registerMessageHandler(uint16_t nMessageID, bool(T::*handler)(uint16_t, CMessage));

		static inline void	registerForwardHandler(uint16_t nMessageID, bool(T::*handler)(SClientSessionInfo, CMessage));

	protected:
		static inline bool	dispatch(T* pObject, uint16_t nFrom, CMessage& pMessage);

		static inline bool	forward(T* pObject, SClientSessionInfo& sSession, CMessage& pMessage);

	private:
		static std::map<uint16_t, funMessageHandler>	s_mapMessageHandler;
		static std::map<uint16_t, funForwardHandler>	s_mapForwardHandler;
	};
}

#define DEFEND_NODE_MESSAGE_FUNCTION(Class) \
		inline bool	onDefaultNodeMessageHandler(uint16_t nFrom, core::CMessage pMessage)\
		{\
			return core::CNodeMessageRegistry<Class>::dispatch(this, nFrom, pMessage);\
		}\
		inline bool	onDefaultNodeForwardHandler(core::SClientSessionInfo& sClientSessionInfo, core::CMessage pMessage)\
		{\
			return core::CNodeMessageRegistry<Class>::forward(this, sClientSessionInfo, pMessage);\
		}

#define REGISTER_NODE_MESSAGE_HANDLER(Class, id, handler)	do { core::CNodeMessageRegistry<Class>::registerMessageHandler(id, (core::CNodeMessageRegistry<Class>::funMessageHandler)handler); core::CCoreServiceApp::Inst()->registerMessageHandler(id, std::bind(&Class::onDefaultNodeMessageHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)
#define REGISTER_NODE_FORWARD_HANDLER(Class, id, handler)	do { core::CNodeMessageRegistry<Class>::registerForwardHandler(id, (core::CNodeMessageRegistry<Class>::funForwardHandler)handler); core::CCoreServiceApp::Inst()->registerForwardHandler(id, std::bind(&Class::onDefaultNodeForwardHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)

#include "node_message_registry.inl"