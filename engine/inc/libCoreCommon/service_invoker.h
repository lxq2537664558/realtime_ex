#pragma once
#include "libBaseCommon/noncopyable.h"
#include "google/protobuf/message.h"

#include "core_common.h"
#include "future.h"

#include <vector>

namespace core
{
	class CServiceBaseImpl;
	class CServiceInvoker :
		public base::noncopyable
	{
	public:
		CServiceInvoker(CServiceBaseImpl* pServiceBaseImpl);
		~CServiceInvoker();

		bool				init();

		bool				send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage);

		bool				broadcast(uint16_t nServiceType, const google::protobuf::Message* pMessage);
		
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);

		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		bool				forward(EMessageTargetType eType, uint64_t nID, uint64_t nSessionID, const google::protobuf::Message* pMessage);
		
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		static bool			send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);

		static bool			broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
	
	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}

#include "service_invoker.inl"