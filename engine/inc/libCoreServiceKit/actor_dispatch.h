#pragma once
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/coroutine.h"

#include <map>

namespace core
{
	template<class T>
	class CActorDispatch
	{
	public:
		typedef void(T::*funMessageHandler)(uint64_t, CMessage);
		typedef void(T::*funForwardHandler)(SClientSessionInfo, CMessage);

	public:
		static inline void registerMessageHandler(uint16_t nMessageID, void(T::*handler)(uint64_t, CMessage), bool bAsync);

		static inline void registerForwardHandler(uint16_t nMessageID, void(T::*handler)(SClientSessionInfo, CMessage), bool bAsync);

		static inline void dispatch(T* pObject, uint64_t nFrom, uint8_t nMessageType, CMessage& pMessage);

		static inline void forward(SClientSessionInfo& sSession, uint8_t nMessageType, CMessage& pMessage);

	private:
		struct SMessageHandlerInfo
		{
			funMessageHandler   handler;
			bool				bAsync;
		};

		struct SForwardHandlerInfo
		{
			funForwardHandler   handler;
			bool				bAsync;
		};

	private:
		static std::map<uint16_t, SMessageHandlerInfo>&	getMessageHandlerInfo()
		{
			static std::map<uint16_t, SMessageHandlerInfo> s_mapMessageHandlerInfo;

			return s_mapMessageHandlerInfo;
		}
		static std::map<uint16_t, SForwardHandlerInfo>&	getForwardHandlerInfo()
		{
			static std::map<uint16_t, SForwardHandlerInfo> s_mapForwardHandlerInfo;

			return s_mapForwardHandlerInfo;
		}
	};
}

#define REGISTER_MESSAGE_HANDLER(id, handler, _async)	registerMessageHandler(id, (funMessageHandler)handler, _async)
#define REGISTER_FORWARD_HANDLER(id, handler, _async)	registerForwardHandler(id, (funForwardHandler)handler, _async)

#include "actor_dispatch.inl"