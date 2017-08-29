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


		//==================================调用指定服务方法=======================================//
		/**
		@brief: 单向的给目标服务发消息
		*/
		bool				send(uint32_t nServiceID, const google::protobuf::Message* pMessage);
		/**
		@brief: 给某个类型的服务广播消息
		*/
		void				broadcast(const std::string& szServiceType, const google::protobuf::Message* pMessage);
		/**
		@brief: 通过callback的方式进行远程调用，调用的时候请用返回消息类型来实例化模板函数
		*/
		template<class T>
		inline void			async_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 通过future的方式进行远程调用
		*/
		template<class T>
		inline void			async_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 同步的调用远程的接口
			通过pResponseMessage来拿到响应结果，这里用shared_ptr的原因是为了自动释放pResponseMessage
			另外 同步调用只支持调用其他服务，不支持调用其他actor
		*/
		template<class T>
		inline uint32_t		sync_invoke(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================调用指定服务方法=======================================//



		//==================================调用指定服务类型方法=======================================//
		/**
		@brief: 单向的给目标服务发消息
		*/
		bool				send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);
		/**
		@brief: 通过callback的方式进行远程调用，调用的时候请用返回消息类型来实例化模板函数
		*/
		template<class T>
		inline void			async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 通过future的方式进行远程调用
		*/
		template<class T>
		inline void			async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 同步的调用远程的接口
		通过pResponseMessage来拿到响应结果，这里用shared_ptr的原因是为了自动释放pResponseMessage
		另外 同步调用只支持调用其他服务，不支持调用其他actor
		*/
		template<class T>
		inline uint32_t		sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================调用指定服务类型方法=======================================//



		//==================================调用指定actor方法=======================================//
		/**
		@brief: 单向的给目标actor发消息
		*/
		bool				send_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage);
		/**
		@brief: 通过callback的方式进行远程调用，调用的时候请用返回消息类型来实例化模板函数
		*/
		template<class T>
		inline void			async_invoke_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 通过future的方式进行远程调用
		*/
		template<class T>
		inline void			async_invoke_a(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================调用指定actor方法=======================================//


		
		/**
		@brief: 单向的给目标（服务，actor）发消息，消息是原始消息，框架不负责序列化
		*/
		bool				gate_forward(uint64_t nSessionID, uint32_t nToServiceID, uint64_t nToActorID, const message_header* pData);
		
		/**
		@brief: 响应rpc请求
		*/
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage, uint32_t nErrorCode = eRRT_OK);


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
		bool				invoke(uint32_t nServiceID, uint64_t nActorID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, CServiceInvokeHolder* pServiceInvokeHolder);
	
	private:
		CServiceBase*	m_pServiceBase;
	};

}

#include "service_invoker.inl"