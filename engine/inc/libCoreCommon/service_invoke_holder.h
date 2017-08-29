#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"
#include "service_base.h"
#include "service_invoker.h"

#include "google/protobuf/message.h"

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
		bool			send(uint32_t nServiceID, const google::protobuf::Message* pMessage);

		void			broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage);

		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		
		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		
		template<class T>
		inline uint32_t	sync_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);
		//==================================����ָ�����񷽷�=======================================//



		//==================================����ָ���������ͷ���=======================================//
		bool			send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);

		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		
		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		
		template<class T>
		inline uint32_t	sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);
		//==================================����ָ���������ͷ���=======================================//



		//==================================����ָ��actor����=======================================//
		bool				send_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage);
		
		template<class T>
		inline void			async_invoke_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		
		template<class T>
		inline void			async_invoke_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================����ָ��actor����=======================================//



		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage, uint32_t nErrorCode = eRRT_OK);

		uint64_t			getHolderID() const;
		CServiceBase*		getServiceBase() const;

	private:
		CServiceBase*	m_pServiceBase;
		uint64_t		m_nID;
	};
}

#include "service_invoke_holder.inl"