#pragma once

#include <memory>

#include "base_object.h"
#include "coroutine.h"
#include "future.h"
#include "ticker.h"

#include "google/protobuf/message.h"

namespace core
{
	class CActorBaseImpl;
	class CServiceBase;
	class __CORE_COMMON_API__ CActorBase :
		public CBaseObject
	{
		friend class CServiceBase;

	protected:
		CActorBase();
		virtual ~CActorBase();

	public:
		virtual void		onInit(const std::string& szContext) { }
		virtual void		onDestroy() { }

		uint64_t			getID() const;

		CServiceBase*		getServiceBase() const;

		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void				unregisterTicker(CTicker* pTicker);

		/*
		给某一个目标发送一个消息
		*/
		bool				send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage);

		/*
		异步的调用远程的接口，通过callback来拿到响应结果
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		/*
		异步的调用远程的接口，通过CFuture来拿到响应结果
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		/*
		同步的调用远程的接口，通过pResponseMessage来拿到响应结果，这里用shared_ptr的原因是为了自动释放pResponseMessage
		另外 同步调用只支持调用其他服务，不支持调用其他actor
		*/
		template<class T>
		inline uint32_t		sync_call(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);


		bool				send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);

		template<class T>
		inline bool			async_call(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);

		template<class T>
		inline bool			async_call(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		/*
		通过请求的session响应请求
		*/
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		virtual void		release();
		/**
		@brief: 发送消息给客户端，这里不要求是protobuf格式的，但是需要在逻辑层自己序列化好
		*/
		bool				send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: 广播消息给客户端，这里不要求是protobuf格式的，但是需要在逻辑层自己序列化好
		*/
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback);

	private:
		CActorBaseImpl*	m_pActorBaseImpl;
	};
}

#include "actor_base.inl"