#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"
#include "service_selector.h"
#include "service_base.h"

namespace core
{
	class CServiceBase;
	class CServiceInvokeHolder;
	class __CORE_COMMON_API__ CServiceInvoker :
		public base::noncopyable
	{
	public:
		CServiceInvoker(CServiceBase* pServiceBase);
		~CServiceInvoker();


		//==================================����ָ�����񷽷�=======================================//
		/**
		@brief: ����ĸ�Ŀ�������Ϣ
		*/
		bool			send(uint32_t nServiceID, const void* pMessage, uint8_t nMessageSerializerType = 0);
		/**
		@brief: ��ĳ�����͵ķ���㲥��Ϣ
		*/
		void			broadcast(const std::string& szServiceType, const void* pMessage, uint8_t nMessageSerializerType = 0);
		/**
		@brief: ͨ��callback�ķ�ʽ����Զ�̵��ã����õ�ʱ�����÷�����Ϣ������ʵ����ģ�庯��
		*/
		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͨ��future�ķ�ʽ����Զ�̵���
		*/
		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͬ���ĵ���Զ�̵Ľӿ�
			ͨ��pResponseMessage���õ���Ӧ�����������shared_ptr��ԭ����Ϊ���Զ��ͷ�pResponseMessage
		*/
		template<class T>
		inline uint32_t	sync_invoke(uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================����ָ�����񷽷�=======================================//



		//==================================����ָ���������ͷ���=======================================//
		/**
		@brief: ����ĸ�Ŀ�������Ϣ
		*/
		bool			send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, uint8_t nMessageSerializerType = 0);
		/**
		@brief: ͨ��callback�ķ�ʽ����Զ�̵��ã����õ�ʱ�����÷�����Ϣ������ʵ����ģ�庯��
		*/
		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͨ��future�ķ�ʽ����Զ�̵���
		*/
		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, CFuture<T>& sFuture, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͬ���ĵ���Զ�̵Ľӿ�
			ͨ��pResponseMessage���õ���Ӧ�����������shared_ptr��ԭ����Ϊ���Զ��ͷ�pResponseMessage
		*/
		template<class T>
		inline uint32_t	sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, std::shared_ptr<T>& pResponseMessage, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================����ָ���������ͷ���=======================================//


		
		/**
		@brief: ����ĸ�Ŀ�������Ϣ����Ϣ��ԭʼ��Ϣ����ܲ��������л�
		*/
		bool			gate_forward(uint64_t nSessionID, uint32_t nToServiceID, const message_header* pData);
		
		/**
		@brief: ��Ӧrpc����
		*/
		void			response(const SSessionInfo& sSessionInfo, const void* pMessage, uint32_t nErrorCode = eRRT_OK, uint8_t nMessageSerializerType = 0);


		//==================================����Ϣ���ͻ���=======================================//
		/**
		@brief: ������Ϣ���ͻ���
		*/
		bool			send(const SClientSessionInfo& sClientSessionInfo, const void* pMessage);
		/**
		@brief: �㲥��Ϣ���ͻ���
		*/
		bool			broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const void* pMessage);
		//==================================����Ϣ���ͻ���=======================================//


	private:
		bool			invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<void>, uint32_t)>& callback, uint8_t nMessageSerializerType, CServiceInvokeHolder* pServiceInvokeHolder);

		bool			send(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, uint8_t nMessageSerializerType);

		template<class T>
		inline void		async_invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, uint8_t nMessageSerializerType, CServiceInvokeHolder* pServiceInvokeHolder);

		template<class T>
		inline void		async_invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, uint8_t nMessageSerializerType, CServiceInvokeHolder* pServiceInvokeHolder);

		template<class T>
		inline uint32_t	sync_invoke(bool bCheckHealth, uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, uint8_t nMessageSerializerType, CServiceInvokeHolder* pServiceInvokeHolder);

	private:
		CServiceBase*	m_pServiceBase;
	};

}

#include "service_invoker.inl"