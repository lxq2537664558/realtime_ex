#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"
#include "service_base.h"
#include "service_invoker.h"

namespace core
{
	/*
	这是一个帮助类，为了服务调用捕获了this或者一些对象级别生命周期的对象，在对象销毁后还有未返回的rpc响应的情况
	*/
	class __CORE_COMMON_API__ CServiceInvokeHolder
	{
	public:
		CServiceInvokeHolder(CServiceBase* pServiceBase);
		virtual ~CServiceInvokeHolder();

		CServiceInvokeHolder(CServiceInvokeHolder& rhs);
		CServiceInvokeHolder& operator = (CServiceInvokeHolder& rhs);

		//==================================调用指定服务方法=======================================//
		bool			send(uint32_t nServiceID, const void* pMessage, uint8_t nMessageSerializerType = 0);

		void			broadcast(const std::string& szServiceType, const void* pMessage, uint8_t nMessageSerializerType = 0);

		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, uint8_t nMessageSerializerType = 0);
		
		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, uint8_t nMessageSerializerType = 0);
		
		template<class T>
		inline uint32_t	sync_invoke(uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, uint8_t nMessageSerializerType = 0);
		//==================================调用指定服务方法=======================================//



		//==================================调用指定服务类型方法=======================================//
		bool			send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, uint8_t nMessageSerializerType = 0);

		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, uint8_t nMessageSerializerType = 0);
		
		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, CFuture<T>& sFuture, uint8_t nMessageSerializerType = 0);
		
		template<class T>
		inline uint32_t	sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, std::shared_ptr<T>& pResponseMessage, uint8_t nMessageSerializerType = 0);
		//==================================调用指定服务类型方法=======================================//


		void			response(const SSessionInfo& sSessionInfo, const void* pMessage, uint32_t nErrorCode = eRRT_OK, uint8_t nMessageSerializerType = 0);

		uint64_t		getHolderID() const;
		CServiceBase*	getServiceBase() const;

	private:
		CServiceBase*	m_pServiceBase;
		uint64_t		m_nID;
	};
}

#include "service_invoke_holder.inl"