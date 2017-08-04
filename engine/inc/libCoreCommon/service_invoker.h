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


		//==================================指定服务之间调用=======================================//
		/**
		@brief: 单向的给目标（服务，actor）发消息
		*/
		bool				send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage);
		/**
		@brief: 通过callback的方式进行远程调用，调用的时候请用返回消息类型来实例化模板函数
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		/**
		@brief: 通过future的方式进行远程调用
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		//==================================指定服务之间调用=======================================//



		//==================================指定服务类型之间调用=======================================//
		/**
		@brief: 单向的给目标（服务，actor）发消息
		*/
		bool				send(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);
		/**
		@brief: 通过callback的方式进行远程调用，调用的时候请用返回消息类型来实例化模板函数
		*/
		template<class T>
		inline bool			async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		/**
		@brief: 通过future的方式进行远程调用
		*/
		template<class T>
		inline bool			async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);
		//==================================指定服务类型之间调用=======================================//

		
		/**
		@brief: 转发客户端消息到目标服务或者actor
		*/
		bool				forward(EMessageTargetType eType, uint64_t nID, const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: 单向的给目标（服务，actor）发消息，消息是原始消息，框架不负责序列化
		*/
		bool				gate_forward(uint64_t nSessionID, uint64_t nSocketID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData);
		/**
		@brief: 响应rpc请求
		*/
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);


		//==================================发消息给客户端=======================================//
		/**
		@brief: 发送消息给客户端
		*/
		bool				send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: 广播消息给客户端
		*/
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);
		//==================================发消息给客户端=======================================//


	private:
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback);

	private:
		CServiceBase*	m_pServiceBase;
	};
}

#include "service_invoker.inl"