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
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);
		/*
		�첽�ĵ���Զ�̵Ľӿڣ�ͨ��CFuture���õ���Ӧ���
		*/
		template<class T>
		inline bool			async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		/*
		ͬ���ĵ���Զ�̵Ľӿڣ�ͨ��pResponseMessage���õ���Ӧ�����������shared_ptr��ԭ����Ϊ���Զ��ͷ�pResponseMessage
		���� ͬ������ֻ֧�ֵ����������񣬲�֧�ֵ�������actor
		*/
		template<class T>
		inline uint32_t		sync_call(uint32_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);


		bool				send(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, google::protobuf::Message* pMessage);

		template<class T>
		inline bool			async_call(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, const std::function<void(const T*, uint32_t)>& callback);

		template<class T>
		inline bool			async_call(const std::string& szServiceType, uint32_t nServiceSelectorType, uint64_t nServiceSelectorContext, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		/*
		ͨ�������session��Ӧ����
		*/
		void				response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);

		virtual void		release();
		/**
		@brief: ������Ϣ���ͻ��ˣ����ﲻҪ����protobuf��ʽ�ģ�������Ҫ���߼����Լ����л���
		*/
		bool				send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/**
		@brief: �㲥��Ϣ���ͻ��ˣ����ﲻҪ����protobuf��ʽ�ģ�������Ҫ���߼����Լ����л���
		*/
		bool				broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);

	private:
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(std::shared_ptr<google::protobuf::Message>&, uint32_t)>& callback);

	private:
		CActorBaseImpl*	m_pActorBaseImpl;
	};
}

#include "actor_base.inl"