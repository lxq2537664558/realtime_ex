#pragma once
#include "libBaseCommon/singleton.h"
#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

namespace core
{
	class CClusterInvoker :
		public base::CSingleton<CClusterInvoker>
	{
	public:
		CClusterInvoker();
		~CClusterInvoker();

		bool				init();

		bool				invoke(uint16_t nNodeID, const void* pData);

		template<class T>
		inline bool			invoke_r(uint16_t nNodeID, const void* pData, CFuture<CMessagePtr<T>>& sFuture);

		template<class T>
		inline bool			invoke_r(uint16_t nNodeID, const void* pData, const std::function<void(CMessagePtr<T>, uint32_t)>& callback);
		
		void				response(const void* pData);
		
		void				response(const SNodeSessionInfo& sNodeSessionInfo, const void* pData);
		
		SNodeSessionInfo	getServiceSessionInfo();
		
		bool				send(const SClientSessionInfo& sClientSessionInfo, const void* pData);
		
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const void* pData);
		
		bool				forward(uint16_t nNodeID, uint64_t nSessionID, const void* pData);
		
		bool				forward_a(uint64_t nActorID, uint64_t nSessionID, const void* pData);
		
	private:
		bool				invokeImpl(uint16_t nNodeID, const void* pData, const std::function<void(CMessagePtr<char>, uint32_t)>& callback);
	};
}

#include "cluster_invoker.inl"