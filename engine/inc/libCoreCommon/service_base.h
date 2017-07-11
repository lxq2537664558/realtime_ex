#pragma once

#include "core_common.h"
#include "ticker.h"
#include "service_invoker.h"
#include "base_object.h"

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

	class CLogicRunnable;
	class CCoreApp;
	/**
	@brief: ���������
	*/
	class CServiceBase :
		public CBaseObject
	{
		friend class CLogicRunnable;
		friend class CCoreApp;

	public:
		CServiceBase();
		virtual ~CServiceBase();

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
		@brief: ע����ͨ�ڵ���Ϣ
		*/
		void				registerMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback);
		/**
		@brief: ע�ᾭ���ؽڵ�ת���ͻ��˵���Ϣ
		*/
		void				registerForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback);
		/**
		@brief: ��ȡ���������
		*/
		CServiceInvoker*	getServiceInvoker() const;
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: ��ȡдbuf���󣬵���Ҫ������Ϣ���
		*/
		base::CWriteBuf&	getWriteBuf() const;
		/*
		@brief: ��ȡQPS
		*/
		uint32_t			getQPS() const;
		/*
		@brief: ��ȡ����״̬
		*/
		EServiceRunState	getState() const;
		/*
		@brief: �߼������˳�
		*/
		void				doQuit();

	protected:
		virtual bool		onInit() = 0;
		virtual void		onFrame() { }
		virtual void		onQuit() = 0;

	private:
		EServiceRunState	m_eState;
		uint16_t			m_nServiceID;
		CServiceInvoker*	m_pServiceInvoker;
	};
}