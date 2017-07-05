#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/core_common.h"
#include "google/protobuf/message.h"

#include "core_service_kit_common.h"

namespace core
{
	class CServiceInvoker :
		public base::noncopyable
	{
	public:
		CServiceInvoker(uint16_t nServiceID);
		~CServiceInvoker();

		bool				init();

		bool				send(uint16_t nServiceID, const google::protobuf::Message* pMessage);

		bool				broadcast(uint16_t nServiceType, const google::protobuf::Message* pMessage);

		template<class T>
		inline bool			async_call(uint16_t nServiceID, const void* pData, CFuture<T>& sFuture);

		template<class T>
		inline bool			async_call(uint16_t nServiceID, const void* pData, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		
		void				response(const google::protobuf::Message* pMessage);
		
		void				response(const SServiceSessionInfo& sServiceSessionInfo, const google::protobuf::Message* pMessage);
		
		bool				send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);
		
		bool				forward(uint16_t nServiceID, uint64_t nSessionID, const google::protobuf::Message* pMessage);
		
		bool				forward_a(uint64_t nActorID, uint64_t nSessionID, const google::protobuf::Message* pMessage);
		
	private:
		bool				invoke(uint16_t nServiceID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
	
	private:
		uint16_t	m_nServiceID;
	};
}

#include "service_invoker.inl"