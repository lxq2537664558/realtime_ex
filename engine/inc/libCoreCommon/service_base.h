#pragma once

#include "core_common.h"
#include "ticker.h"
#include "service_id_converter.h"
#include "message_serializer.h"
#include "service_selector.h"

#include "libBaseCommon/buf_file.h"

#include <set>

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

	class CServiceInvoker;
	class CServiceInvokeHolder;
	class CCoreService;
	class CCoreServiceMgr;
	class CBaseConnectionMgr;
	/**
	@brief: ���������
	*/
	class __CORE_COMMON_API__ CServiceBase :
		public base::noncopyable
	{
		friend class CCoreService;
		friend class CServiceInvoker;
		friend class CServiceInvokeHolder;
		friend class CCoreServiceMgr;

	public:
		CServiceBase(const SServiceBaseInfo& sServiceBaseInfo, const std::string& szConfigFileName);
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
		void				setServiceSelector(uint32_t nType, CServiceSelector* pServiceSelector);
		/**
		@brief: ��ȡ����ѡ����
		*/
		CServiceSelector*	getServiceSelector(uint32_t nType) const;

		/**
		@brief: �жϷ����Ƿ񽡿�
		*/
		bool				isServiceHealth(uint32_t nServiceID) const;

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
		@brief: ��ȡ���ӹ�����
		*/
		CBaseConnectionMgr*	getBaseConnectionMgr() const;
		/**
		@brief: ע����ͨ������Ϣ
		*/
		void				registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const void*)>& callback);
		/**
		@brief: ע�ᾭ���ط���ת���ͻ��˵ķ�����Ϣ
		*/
		void				registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const void*)>& callback);
		
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
		@brief: ����ToGate��Ϣ�ص�
		*/
		void				setToGateMessageCallback(const std::function<void(uint64_t, const void*, uint16_t)>& callback);
		/**
		@brief: ����ToGate�㲥��Ϣ�ص�
		*/
		void				setToGateBroadcastMessageCallback(const std::function<void(const uint64_t*, uint16_t, const void*, uint16_t)>& callback);
		
		/**
		@brief: ��ȡ���������
		*/
		CServiceInvoker*	getServiceInvoker() const;
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
		@brief: �����Ϣ�����л���
		*/
		void				addServiceMessageSerializer(CMessageSerializer* pMessageSerializer);
		/**
		@brief: ����ĳһ��service��ʹ�õ����л���
		*/
		void				setServiceMessageSerializer(uint32_t nServiceID, uint32_t nType);
		/**
		@brief: ����forward��Ϣ�����л���
		*/
		void				setForwardMessageSerializer(CMessageSerializer* pMessageSerializer);
		/**
		@brief: ��ȡĳһ����ָ������ͨѶʱ��Ϣ�����л���
		*/
		CMessageSerializer*	getServiceMessageSerializer(uint32_t nServiceID) const;
		/**
		@brief: ��ȡĳһ����ָ������ͨѶʱ��Ϣ�����л�������
		*/
		uint32_t			getServiceMessageSerializerType(uint32_t nServiceID) const;

		/**
		@brief: ��ȡforward��Ϣ�����л���
		*/
		CMessageSerializer*	getForwardMessageSerializer() const;
		/**
		@brief: ����service_idת����
		*/
		virtual	CServiceIDConverter*
							getServiceIDConverter() { return nullptr; }

		/**
		@brief: ���ݽڵ����ֻ�ȡ�ڵ�id
		*/
		uint32_t			getServiceID(const std::string& szName) const;
		/**
		@brief: ���ݷ�������ȡ�������ͷ�������з���id
		*/
		const std::set<uint32_t>&
							getServiceIDByType(const std::string& szName) const;
		/**
		@brief: ���ݷ�������ȡ�������ͷ�������м���ķ���id
		*/
		const std::vector<uint32_t>&
							getActiveServiceIDByType(const std::string& szName) const;

		uint32_t			getQPS() const;

		virtual void		release() = 0;

	protected:
		virtual bool		onInit() = 0;
		virtual void		onFrame() { }
		virtual void		onQuit() = 0;

	private:
		CCoreService*	m_pCoreService;
	};
}