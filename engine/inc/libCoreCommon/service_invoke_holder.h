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
	����һ�������࣬Ϊ�˷�����ò�����this����һЩ���󼶱��������ڵĶ����ڶ������ٺ���δ���ص�rpc��Ӧ�����
	*/
	class __CORE_COMMON_API__ CServiceInvokeHolder
	{
	public:
		CServiceInvokeHolder(CServiceBase* pServiceBase);
		virtual ~CServiceInvokeHolder();

		CServiceInvokeHolder(CServiceInvokeHolder& rhs);
		CServiceInvokeHolder& operator = (CServiceInvokeHolder& rhs);

		//==================================����ָ�����񷽷�=======================================//
		bool			send(uint32_t nServiceID, const void* pMessage, const SInvokeOption* pInvokeOption = nullptr);

		void			broadcast(const std::string& szServiceType, const void* pMessage, const SInvokeOption* pInvokeOption = nullptr);

		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, const SInvokeOption* pInvokeOption = nullptr);
		
		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, const SInvokeOption* pInvokeOption = nullptr);
		
		template<class T>
		inline uint32_t	sync_invoke(uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, const SInvokeOption* pInvokeOption = nullptr);
		//==================================����ָ�����񷽷�=======================================//



		//==================================����ָ���������ͷ���=======================================//
		bool			send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, const SInvokeOption* pInvokeOption = nullptr);

		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, const SInvokeOption* pInvokeOption = nullptr);
		
		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, CFuture<T>& sFuture, const SInvokeOption* pInvokeOption = nullptr);
		
		template<class T>
		inline uint32_t	sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, std::shared_ptr<T>& pResponseMessage, const SInvokeOption* pInvokeOption = nullptr);
		//==================================����ָ���������ͷ���=======================================//


		void			response(const SSessionInfo& sSessionInfo, const void* pMessage, uint32_t nErrorCode = eRRT_OK, uint8_t nMessageSerializerType = 0);

		uint64_t		getHolderID() const;
		CServiceBase*	getServiceBase() const;

	private:
		CServiceBase*	m_pServiceBase;
		uint64_t		m_nHolderID;
	};
}

#include "service_invoke_holder.inl"