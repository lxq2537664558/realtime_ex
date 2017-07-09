#pragma once
#include "libBaseCommon/noncopyable.h"
#include "google/protobuf/message.h"

#include "coroutine.h"
#include "core_service_kit_common.h"

namespace core
{
	class CActorBaseImpl;
	class CActorFactory;
	class CActorBase :
		public base::noncopyable
	{
	protected:
		CActorBase();
		virtual ~CActorBase();

	public:
		virtual bool		onInit(void* pContext) { return true; }
		virtual void		onDestroy() { }

		uint64_t			getID() const;

		/*
		��ĳһ��actor������Ϣ
		*/
		bool				send(uint64_t nID, const google::protobuf::Message* pMessage);

		/*
		�첽�ĵ���Զ�̵Ľӿڣ�ͨ��callback���õ���Ӧ���
		*/
		template<class T>
		bool				async_call(uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
		/*
		�첽�ĵ���Զ�̵Ľӿڣ�ͨ��CFuture���õ���Ӧ���
		*/
		template<class T>
		inline bool			async_call(uint64_t nID, const google::protobuf::Message* pMessage, CFuture<T>& sFuture);

		/*
		ͬ���ĵ���Զ�̵Ľӿڣ�ͨ��pResponseMessage���õ���Ӧ�����������shared_ptr��ԭ����Ϊ���Զ��ͷ�pResponseMessage
		*/
		template<class T>
		inline uint32_t		sync_call(uint64_t nID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);

		void				response(const google::protobuf::Message* pMessage);
		void				response(const SActorSessionInfo& sActorSessionInfo, const google::protobuf::Message* pMessage);

		SActorSessionInfo	getActorSessionInfo() const;

		void				release();

		static void			registerMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SActorSessionInfo, const google::protobuf::Message*)>& handler);
		static void			registerForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler);

		static CActorBase*	createActorBase(void* pContext, CActorFactory* pActorBaseFactory);

		static uint16_t		getServiceID(uint64_t nActorID);
		
	private:
		bool				invoke(uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);

	private:
		CActorBaseImpl*	m_pActorBaseImpl;
	};
}

#include "actor_base.inl"