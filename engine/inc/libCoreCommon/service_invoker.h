#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libBaseCommon/debug_helper.h"

#include "core_common.h"
#include "future.h"
#include "promise.h"

#include "google/protobuf/message.h"

namespace core
{
	class CServiceBase;
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
		@brief: ͨ��callback�ķ�ʽ����Զ�̵��ã����õ�ʱ�����÷�����Ϣ������ʵ����ģ�庯��
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		/**
		@brief: ͨ��future�ķ�ʽ����Զ�̵���
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		//==================================ָ������֮�����=======================================//



		//==================================ָ����������֮�����=======================================//
		/**
		@brief: ����ĸ�Ŀ�꣨����actor������Ϣ
		*/
		bool				send(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);
		/**
		@brief: ͨ��callback�ķ�ʽ����Զ�̵��ã����õ�ʱ�����÷�����Ϣ������ʵ����ģ�庯��
		*/
		template<class T>
		inline bool			async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		/**
		@brief: ͨ��future�ķ�ʽ����Զ�̵���
		*/
		template<class T>
		inline bool			async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		//==================================ָ����������֮�����=======================================//

		
		/**
		@brief: ת���ͻ�����Ϣ��Ŀ��������actor
		*/
		bool				forward(EMessageTargetType eType, uint64_t nID, const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: ����ĸ�Ŀ�꣨����actor������Ϣ����Ϣ��ԭʼ��Ϣ����ܲ��������л�
		*/
		bool				gate_forward(uint64_t nSessionID, uint64_t nSocketID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData);
		/**
		@brief: ��Ӧrpc����
		*/
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);


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
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback);

	private:
		CServiceBase*	m_pServiceBase;
	};
}

#include "service_invoker.inl"