#pragma once

#include "core_common.h"
#include "ticker.h"
#include "actor_id_converter.h"
#include "service_id_converter.h"
#include "protobuf_factory.h"
#include "service_selector.h"
#include "actor_factory.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	// ����״̬ eSRS_Start->eSRS_Normal->eSRS_Quitting->eSRS_Quit
	// ����eSRS_Quitting״̬�л���eSRS_Quit�����߼�����������doQuit����ɵģ�
	// ��������֤�˹ط�ǰ��һЩ��Ҫ�Ƚϳ���ʱ����ȷ��һЩ�µ���Ϊ���������ݴ洢
	enum EServiceRunState
	{
		eSRS_Start		= 0,	// ����״̬
		eSRS_Normal		= 1,	// ����״̬
		eSRS_Quitting	= 2,	// �˳���
		eSRS_Quit		= 3,	// �����˳�
	};

	class CActorBase;
	class CServiceInvoker;
	class CCoreService;
	/**
	@brief: ���������
	*/
	class __CORE_COMMON_API__ CServiceBase :
		public base::noncopyable
	{
		friend class CCoreService;
		friend class CServiceInvoker;
		
	public:
		CServiceBase();
		virtual ~CServiceBase();

		uint32_t			getServiceID() const;
		/**
		@brief: ��ȡ���������Ϣ
		*/
		const SServiceBaseInfo&
							getServiceBaseInfo() const;

		/**
		@brief: ��ӷ���ѡ����
		*/
		void				setServiceSelector(const std::string& szType, CServiceSelector* pServiceSelector);
		/**
		@brief: ��ȡ����ѡ����
		*/
		CServiceSelector*	getServiceSelector(const std::string& szType) const;

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

		/**
		@brief: ע����ͨ������Ϣ
		*/
		void				registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		/**
		@brief: ע�ᾭ���ط���ת���ͻ��˵ķ�����Ϣ
		*/
		void				registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);
		
		/**
		@brief: ע����ͨactor��Ϣ
		*/
		void				registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		/**
		@brief: ע�ᾭ���ط���ת���ͻ��˵�actor��Ϣ
		*/
		void				registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);

		/**
		@brief: ��ȡ�ͻ���ת����Ϣ����
		*/
		const std::string&	getForwardMessageName(uint32_t nMessageID);

		/**
		@brief: ����ȫ�ֵķ������ӳɹ��ص�
		*/
		void				setServiceConnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);
		/**
		@brief: ����ȫ�ֵķ������ӶϿ��ص�
		*/
		void				setServiceDisconnectCallback(const std::function<void(const std::string&, uint32_t)>& callback);

		/**
		@brief: ��ȡ���������
		*/
		CServiceInvoker*	getServiceInvoker() const;
		/**
		@brief: ����actor
		*/
		CActorBase*			createActor(const std::string& szType, uint64_t nActorID, const std::string& szContext);
		/**
		@brief: ����actor
		*/
		void				destroyActor(CActorBase* pActorBase);
		/**
		@brief: ����id��ȡactor
		*/
		CActorBase*			getActorBase(uint64_t nID) const;
		/*
		@brief: ��ȡ����״̬
		*/
		EServiceRunState	getRunState() const;
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: �߼������˳�
		*/
		void				doQuit();

		/**
		@brief: ��ȡprotobuf������
		*/
		virtual CProtobufFactory*
							getProtobufFactory() const = 0;

		/**
		@brief: ��ȡactor_idת����
		*/
		virtual CActorIDConverter*
							getActorIDConverter() { return nullptr; }
		/**
		@brief: ����service_idת����
		*/
		virtual	CServiceIDConverter*
							getServiceIDConverter() { return nullptr; }
		/**
		@brief: ��ȡactor����
		*/
		virtual CActorFactory*
							getActorFactory(const std::string& szType) const { return nullptr; }

		virtual void		release() = 0;

	protected:
		virtual bool		onInit() = 0;
		virtual void		onFrame() { }
		virtual void		onQuit() = 0;

	private:
		CCoreService*	m_pCoreService;
	};
}