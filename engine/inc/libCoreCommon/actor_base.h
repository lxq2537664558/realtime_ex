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
		��ĳһ��Ŀ�귢��һ����Ϣ
		*/
		bool				send(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage);

		/*
		�첽�ĵ���Զ�̵Ľӿڣ�ͨ��callback���õ���Ӧ���
		*/
		template<class T>
		bool				async_call(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);
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
		inline uint32_t		sync_call(uint16_t nServiceID, const google::protobuf::Message* pMessage, std::shared_ptr<T>& pResponseMessage);

		void				release();

		/*
		ͨ�������session��Ӧ����
		*/
		static void			response(const SSessionInfo& sSessionInfo, const google::protobuf::Message* pMessage);
		/*
		������Ϣ���ͻ���
		*/
		static bool			send(const SClientSessionInfo& sClientSessionInfo, const google::protobuf::Message* pMessage);
		/*
		�㲥��Ϣ���ͻ���
		*/
		static bool			broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const google::protobuf::Message* pMessage);

		static void			registerMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& handler);
		static void			registerForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& handler);

		static CActorBase*	createActor(const std::string& szClassName, void* pContext);

	private:
		bool				invoke(EMessageTargetType eType, uint64_t nID, const google::protobuf::Message* pMessage, uint64_t nCoroutineID, const std::function<void(const google::protobuf::Message*, uint32_t)>& callback);

	private:
		CActorBaseImpl*	m_pActorBaseImpl;
	};
}

#include "actor_base.inl"