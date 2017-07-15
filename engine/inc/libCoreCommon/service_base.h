#pragma once

#include "core_common.h"
#include "ticker.h"
#include "service_invoker.h"
#include "base_object.h"
#include "actor_id_converter.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	// ����״̬ eSRS_Start->eSRS_Normal->eSRS_Quitting->eSRS_Quit
	// ����eSRS_Quitting״̬�л���eSRS_Quit�����߼�����������doQuit����ɵģ�
	// ��������֤�˹ط�ǰ��һЩ��Ҫ�Ƚϳ���ʱ����ȷ��һЩ�µ���Ϊ���������ݴ洢
	enum EServiceRunState
	{
		eSRS_Start = 0,		// ����״̬
		eSRS_Normal = 1,	// ����״̬
		eSRS_Quitting = 2,	// �˳���
		eSRS_Quit = 3,		// �����˳�
	};

	class CActorBase;
	class CServiceBaseImpl;
	/**
	@brief: ���������
	*/
	class CServiceBase :
		public CBaseObject
	{
		friend class CServiceBaseImpl;
		
	public:
		CServiceBase();
		virtual ~CServiceBase();

		/**
		@brief: ��ȡ����ID
		*/
		const SServiceBaseInfo&	getServiceBaseInfo() const;

		/**
		@brief: ����actoridת����
		*/
		void					setActorIDConverter(CActorIDConverter* pActorIDConverter);
		/**
		@brief: ��ȡactoridת����
		*/
		CActorIDConverter*		getActorIDConverter() const;

		/**
		@brief: ע�ᶨʱ��
		nStartTime ��һ�δ�����ʱ����ʱ��
		nIntervalTime ��һ�δ�����ʱ�����������ʱ�������ļ��ʱ�䣬�����ֵ��0�ͱ�ʾ�����ʱ��ֻ����һ��
		*/
		void					registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void					unregisterTicker(CTicker* pTicker);

		/**
		@brief: ע����ͨ������Ϣ
		*/
		void					registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback);
		/**
		@brief: ע�ᾭ���ط���ת���ͻ��˵ķ�����Ϣ
		*/
		void					registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback);
		
		/**
		@brief: ע����ͨactor��Ϣ
		*/
		void					registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		/**
		@brief: ע�ᾭ���ط���ת���ͻ��˵�actor��Ϣ
		*/
		void					registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);

		/**
		@brief: ����ȫ�ֵķ������ӳɹ��ص�
		*/
		void					setServiceConnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: ����ȫ�ֵķ������ӶϿ��ص�
		*/
		void					setServiceDisconnectCallback(const std::function<void(uint16_t)>& callback);

		/**
		@brief: ��ȡ���������
		*/
		CServiceInvoker*		getServiceInvoker() const;
		/**
		@brief: ����actor
		*/
		CActorBase*				createActor(const std::string& szClassName, void* pContext);
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&		getConfigFileName() const;
		/*
		@brief: ��ȡдbuf���󣬵���Ҫ������Ϣ���
		*/
		base::CWriteBuf&		getWriteBuf() const;
		/*
		@brief: ��ȡQPS
		*/
		uint32_t				getQPS() const;
		/*
		@brief: ��ȡ����״̬
		*/
		EServiceRunState		getState() const;
		/*
		@brief: �߼������˳�
		*/
		void					doQuit();

	protected:
		virtual bool			onInit() = 0;
		virtual void			onFrame() { }
		virtual void			onQuit() = 0;

	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}