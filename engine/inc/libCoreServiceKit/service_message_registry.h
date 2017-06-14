#pragma once
#include "base_actor.h"

#include <map>

namespace core
{
	template<class T>
	class CServiceMessageRegistry
	{
	public:
		typedef bool(T::*funMessageHandler)(uint16_t, CMessagePtr<char>);
		typedef bool(T::*funForwardHandler)(SClientSessionInfo, CMessagePtr<char>);

	public:
		static inline void	registerMessageHandler(uint16_t nMessageID, bool(T::*handler)(uint16_t, CMessagePtr<char>));

		static inline void	registerForwardHandler(uint16_t nMessageID, bool(T::*handler)(SClientSessionInfo, CMessagePtr<char>));

	protected:
		static inline bool	dispatch(T* pObject, uint16_t nFrom, CMessagePtr<char>& pMessage);

		static inline bool	forward(T* pObject, SClientSessionInfo& sSession, CMessagePtr<char>& pMessage);

	private:
		static std::map<uint16_t, funMessageHandler>	s_mapMessageHandler;
		static std::map<uint16_t, funForwardHandler>	s_mapForwardHandler;
	};
}

#define DEFEND_SERVICE_MESSAGE_FUNCTION(Class) \
		inline bool	onDefaultServiceMessageHandler(uint16_t nFrom, core::CMessagePtr<char> pMessage)\
		{\
			return core::CServiceMessageRegistry<Class>::dispatch(this, nFrom, pMessage);\
		}\
		inline bool	onDefaultServiceForwardHandler(core::SClientSessionInfo& sClientSessionInfo, core::CMessagePtr<char> pMessage)\
		{\
			return core::CServiceMessageRegistry<Class>::forward(this, sClientSessionInfo, pMessage);\
		}

#define REGISTER_SERVICE_MESSAGE_HANDLER(Class, id, handler)	do { core::CServiceMessageRegistry<Class>::registerMessageHandler(id, (core::CServiceMessageRegistry<Class>::funMessageHandler)handler); core::CCoreServiceApp::Inst()->registerMessageHandler(id, std::bind(&Class::onDefaultServiceMessageHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)
#define REGISTER_SERVICE_FORWARD_HANDLER(Class, id, handler)	do { core::CServiceMessageRegistry<Class>::registerForwardHandler(id, (core::CServiceMessageRegistry<Class>::funForwardHandler)handler); core::CCoreServiceApp::Inst()->registerForwardHandler(id, std::bind(&Class::onDefaultServiceForwardHandler, this, std::placeholders::_1, std::placeholders::_2)); } while(0)

#include "service_message_registry.inl"