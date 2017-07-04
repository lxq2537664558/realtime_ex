#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"
#include "google/protobuf/message.h"

#include "core_service_kit_common.h"


namespace core
{
	class CServiceInvoker :
		public base::CSingleton<CServiceInvoker>
	{
	public:
		CServiceInvoker();
		~CServiceInvoker();

		bool				init();

		bool				send(uint16_t nServiceID, const google::protobuf::Message* pMessage);

		template<class T>
		inline bool			async_call(uint16_t nServiceID, const void* pData, CFuture<CMessagePtr<T>>& sFuture);

		template<class T>
		inline bool			async_call(uint16_t nServiceID, const void* pData, const std::function<void(CMessagePtr<T>, uint32_t)>& callback);
		
		void				response(const void* pData);
		
		void				response(const SServiceSessionInfo& sServiceSessionInfo, const void* pData);
		
		bool				send(const SClientSessionInfo& sClientSessionInfo, const void* pData);
		
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const void* pData);
		
		bool				forward(uint16_t nServiceID, uint64_t nSessionID, const void* pData);
		
		bool				forward_a(uint64_t nActorID, uint64_t nSessionID, const void* pData);
		
	private:
		bool				invokeImpl(uint16_t nServiceID, const void* pData, const std::function<void(CMessagePtr<char>, uint32_t)>& callback);
	};
}

#include "service_invoker.inl"