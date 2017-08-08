#pragma once

#include <memory>

#include "coroutine.h"
#include "future.h"
#include "promise.h"
#include "ticker.h"
#include "service_base.h"
#include "service_invoker.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/noncopyable.h"

#include "google/protobuf/message.h"

namespace core
{
	/*
	actor�Ĵ�������������
	createActor  ---> ����CActorBase���� ---> ����CCoreActor���� ---> ����CActorBase��onInit���������������Э��ȥ���ģ�---> ����actor
	destroyActor ---> ����CActorBase��onDestroy��������ͨ���ã�---> ����CCoreActor���� ---> ����CActorBase��release��������CActorBase����
	*/

	class CCoreActor;
	class CActorInvokeHolder;
	class __CORE_COMMON_API__ CActorBase :
		public base::noncopyable
	{
		friend class CServiceBase;
		friend class CActorInvokeHolder;

	protected:
		CActorBase();
		virtual ~CActorBase();

	public:
		uint64_t			getID() const;

		CServiceBase*		getServiceBase() const;

		/**
		@brief: ע�ᶨʱ��
		nStartTime ��һ�δ�����ʱ����ʱ��
		nIntervalTime ��һ�δ�����ʱ�����������ʱ�������ļ��ʱ�䣬�����ֵ��0�ͱ�ʾ�����ʱ��ֻ����һ��
		*/
		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void				unregisterTicker(CTicker* pTicker);

		/*
		��ĳһ��Ŀ�귢��һ����Ϣ
		*/
		bool				send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage);

		/*
		�첽�ĵ���Զ�̵Ľӿڣ�ͨ��callback���õ���Ӧ���
		*/
		template<class T>
		inline void			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CActorInvokeHolder* pActorInvokeHolder = nullptr);
		/*
		�첽�ĵ���Զ�̵Ľӿڣ�ͨ��CFuture���õ���Ӧ���
		*/
		template<class T>
		inline void			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CActorInvokeHolder* pActorInvokeHolder = nullptr);

		/*
		ͬ���ĵ���Զ�̵Ľӿڣ�ͨ��pResponseMessage���õ���Ӧ�����������shared_ptr��ԭ����Ϊ���Զ��ͷ�pResponseMessage
		���� ͬ������ֻ֧�ֵ����������񣬲�֧�ֵ�������actor
		*/
		template<class T>
		inline uint32_t		sync_call(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage, CActorInvokeHolder* pActorInvokeHolder = nullptr);


		bool				send(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);

		template<class T>
		inline void			async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback, CActorInvokeHolder* pActorInvokeHolder = nullptr);

		template<class T>
		inline void			async_call(const std::string& szServiceType, const std::string& szServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture, CActorInvokeHolder* pActorInvokeHolder = nullptr);

		/*
		ͨ�������session��Ӧ����
		*/
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		/**
		@brief: ������Ϣ���ͻ��ˣ����ﲻҪ����protobuf��ʽ�ģ�������Ҫ���߼����Լ����л���
		*/
		bool				send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: �㲥��Ϣ���ͻ��ˣ����ﲻҪ����protobuf��ʽ�ģ�������Ҫ���߼����Լ����л���
		*/
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		virtual void		onInit(const std::string& szContext) { }
		virtual void		onDestroy() { }

		virtual void		release() = 0;

		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>& callback, CActorInvokeHolder* pActorInvokeHolder);

	private:
		CCoreActor*	m_pCoreActor;
	};
}

#include "actor_base.inl"