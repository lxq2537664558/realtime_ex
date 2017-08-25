#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"
#include "service_selector.h"
#include "service_base.h"

#include "google/protobuf/message.h"

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


		//==================================ָ������֮�����=======================================//
		/**
		@brief: ����ĸ�Ŀ�꣨����actor������Ϣ
		*/
		bool				send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage);
		/**
		@brief: ��ĳ�����͵ķ���㲥��Ϣ
		*/
		void				broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage);
		/**
		@brief: ͨ��callback�ķ�ʽ����Զ�̵��ã����õ�ʱ�����÷�����Ϣ������ʵ����ģ�庯��
		*/
		template<class T>
		inline void			async_invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͨ��future�ķ�ʽ����Զ�̵���
		*/
		template<class T>
		inline void			async_invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͬ���ĵ���Զ�̵Ľӿ�
			ͨ��pResponseMessage���õ���Ӧ�����������shared_ptr��ԭ����Ϊ���Զ��ͷ�pResponseMessage
			���� ͬ������ֻ֧�ֵ����������񣬲�֧�ֵ�������actor
		*/
		template<class T>
		inline uint32_t		sync_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================ָ������֮�����=======================================//



		//==================================ָ����������֮�����=======================================//
		/**
		@brief: ����ĸ�Ŀ�꣨����actor������Ϣ
		*/
		bool				send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);
		/**
		@brief: ͨ��callback�ķ�ʽ����Զ�̵��ã����õ�ʱ�����÷�����Ϣ������ʵ����ģ�庯��
		*/
		template<class T>
		inline void			async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͨ��future�ķ�ʽ����Զ�̵���
		*/
		template<class T>
		inline void			async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: ͬ���ĵ���Զ�̵Ľӿ�
		ͨ��pResponseMessage���õ���Ӧ�����������shared_ptr��ԭ����Ϊ���Զ��ͷ�pResponseMessage
		���� ͬ������ֻ֧�ֵ����������񣬲�֧�ֵ�������actor
		*/
		template<class T>
		inline uint32_t		sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================ָ����������֮�����=======================================//

		
		/**
		@brief: ת���ͻ�����Ϣ��Ŀ��������actor
		*/
		bool				forward(EMessageTargetType eType, uint64_t nID, const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: ����ĸ�Ŀ�꣨����actor������Ϣ����Ϣ��ԭʼ��Ϣ����ܲ��������л�
		*/
		bool				gate_forward(uint64_t nSessionID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData);
		
		/**
		@brief: ��Ӧrpc����
		*/
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage, uint32_t nErrorCode = eRRT_OK);


		//==================================����Ϣ���ͻ���=======================================//
		/**
		@brief: ������Ϣ���ͻ���
		*/
		bool				send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: �㲥��Ϣ���ͻ���
		*/
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);
		//==================================����Ϣ���ͻ���=======================================//


	private:
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder);
	
	private:
		CServiceBase*	m_pServiceBase;
	};
}

#include "service_invoker.inl"