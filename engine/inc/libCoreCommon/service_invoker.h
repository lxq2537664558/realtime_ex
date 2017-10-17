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


		//==================================调用指定服务方法=======================================//
		/**
		@brief: 单向的给目标服务发消息
		*/
		bool			send(uint32_t nServiceID, const void* pMessage, uint8_t nMessageSerializerType = 0);
		/**
		@brief: 给某个类型的服务广播消息
		*/
		void			broadcast(const std::string& szServiceType, const void* pMessage, uint8_t nMessageSerializerType = 0);
		/**
		@brief: 通过callback的方式进行远程调用，调用的时候请用返回消息类型来实例化模板函数
		*/
		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 通过future的方式进行远程调用
		*/
		template<class T>
		inline void		async_invoke(uint32_t nServiceID, const void* pMessage, CFuture<T>& sFuture, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 同步的调用远程的接口
			通过pResponseMessage来拿到响应结果，这里用shared_ptr的原因是为了自动释放pResponseMessage
		*/
		template<class T>
		inline uint32_t	sync_invoke(uint32_t nServiceID, const void* pMessage, std::shared_ptr<T>& pResponseMessage, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================调用指定服务方法=======================================//



		//==================================调用指定服务类型方法=======================================//
		/**
		@brief: 单向的给目标服务发消息
		*/
		bool			send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, uint8_t nMessageSerializerType = 0);
		/**
		@brief: 通过callback的方式进行远程调用，调用的时候请用返回消息类型来实例化模板函数
		*/
		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, const std::function<void(const T*, uint32_t)>& callback, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 通过future的方式进行远程调用
		*/
		template<class T>
		inline void		async_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, CFuture<T>& sFuture, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		/**
		@brief: 同步的调用远程的接口
			通过pResponseMessage来拿到响应结果，这里用shared_ptr的原因是为了自动释放pResponseMessage
		*/
		template<class T>
		inline uint32_t	sync_invoke(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const void* pMessage, std::shared_ptr<T>& pResponseMessage, uint8_t nMessageSerializerType = 0, CServiceInvokeHolder* pServiceInvokeHolder = nullptr);
		//==================================调用指定服务类型方法=======================================//


		
		/**
		@brief: 单向的给目标服务发消息，消息是原始消息，框架不负责序列化
		*/
		bool			gate_forward(uint64_t nSessionID, uint32_t nToServiceID, const message_header* pData);
		
		/**
		@brief: 响应rpc请求
		*/
		void			response(const SSessionInfo& sSessionInfo, const void* pMessage, uint32_t nErrorCode = eRRT_OK, uint8_t nMessageSerializerType = 0);


		//==================================发消息给客户端=======================================//
		/**
		@brief: 发送消息给客户端
		*/
		bool			send(const SClientSessionInfo& sClientSessionInfo, const void* pMessage);
		/**
		@brief: 广播消息给客户端
		*/
		bool			broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const void* pMessage);
		//==================================发消息给客户端=======================================//


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