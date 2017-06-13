#pragma once

#include "core_common.h"
#include "ticker.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	// ������״̬ eARS_Start->eARS_Normal->eARS_Quitting->eARS_Quit
	// ����eARS_Quitting״̬�л���eARS_Quit�����߼�����������doQuit����ɵģ�
	// ��������֤�˹ط�ǰ��һЩ��Ҫ�Ƚϳ���ʱ����ȷ��һЩ�µ���Ϊ���������ݴ洢
	enum EAppRunState
	{
		eARS_Start		= 0,	// ����״̬
		eARS_Normal		= 1,	// ����״̬
		eARS_Quitting	= 2,	// �˳���
		eARS_Quit		= 3,	// �����˳�
	};

	class CBaseConnectionMgr;
	class CCoreConnectionMgr;
	class CCoreApp;
	/**
	@brief: ���������
	*/
	class CBaseApp
	{
		friend class CCoreApp;
		friend class CCoreConnectionMgr;

	public:
		CBaseApp();
		virtual ~CBaseApp();

		static CBaseApp*& Inst();

		/**
		@brief: �������
		*/
		bool						run(int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: ע�ᶨʱ��
		nStartTime ��һ�δ�����ʱ����ʱ��
		nIntervalTime ��һ�δ�����ʱ�����������ʱ�������ļ��ʱ�䣬�����ֵ��0�ͱ�ʾ�����ʱ��ֻ����һ��
		*/
		void						registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void						unregisterTicker(CTicker* pTicker);
		/**
		@brief: ��ȡ��ǰ�߼�ʱ��
		*/
		int64_t						getLogicTime() const;
		/*
		@brief: ��ȡ���ӹ�����
		*/
		CBaseConnectionMgr*			getBaseConnectionMgr() const;
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&			getConfigFileName() const;
		/*
		@brief: ��ȡдbuf���󣬵���Ҫ������Ϣ���
		*/
		base::CWriteBuf&			getWriteBuf() const;
		/*
		@brief: ��ȡQPS
		*/
		uint32_t					getQPS() const;
		/*
		@brief: ��Ƿ�æ�������߼��߳̿��Բ��ȴ���Ϣ���е�����
		*/
		void						busy();
		/*
		@brief: �����Ƿ����������Ϣ
		*/
		void						debugLog(bool bEnable);
		/**
		@brief: �����Ƿ�����ܷ���
		*/
		void						profiling(bool bEnable);
		
	protected:
		virtual bool				onInit() { return true; }
		virtual void				onProcess() { }
		virtual void				onDestroy() { }

		virtual	void				onQuit() = 0;
		void						doQuit();
	};
}