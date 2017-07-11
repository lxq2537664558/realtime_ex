#pragma once
#include "google/protobuf/message.h"

#include "base_object.h"
#include "coroutine.h"
#include "core_common.h"
#include "future.h"

#include <memory>

namespace core
{
	class CActorBaseImpl;
	class CActorBase :
		public CBaseObject
	{
	protected:
		CActorBase();
		virtual ~CActorBase();

	public:
		virtual bool		onInit(void* pContext) { return true; }
		virtual void		onDestroy() { }

		uint64_t			getID() const;

		/*
		给某一个actor发送消息
		*/
		bool				send(uint64_t nID, const google::protobuf::Message* pMessage);

		/*
		异步的调用远程的接口，通过callback来拿到响应结果
		*/
		template<class T>
		bool				async_call(uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		/*
		异步的调用远程的接口，通过CFuture来拿到响应结果
		*/
		template<class T>
		inline bool			async_call(uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		/*
		同步的调用远程的接口，通过pResponseMessage来拿到响应结果，这里用shared_ptr的原因是为了自动释放pResponseMessage
		*/
		template<class T>
		inline uint32_t		sync_call(uint64_t nID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);

		void				response(const SActorSessionInfo& sActorSessionInfo, const google::protobuf::Message* pMessage);

		void				release();

		static void			registerMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>& handler);
		static void			registerForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler);

		static CActorBase*	createActor(const std::string& szClassName, void* pContext);

	private:
		bool				invoke(uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);

	private:
		CActorBaseImpl*	m_pActorBaseImpl;
	};
}

#include "actor_base.inl"