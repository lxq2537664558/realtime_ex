#pragma once

#include "core_common.h"
#include "service_base.h"
#include "actor_id_converter.h"
#include "ticker.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	class CBaseConnectionMgr;
	class CCoreApp;
	/**
	@brief: ���������
	*/
	class CBaseApp
	{
		friend class CCoreApp;
		
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
		void						registerTicker(uint8_t nType, uint16_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void						unregisterTicker(CTicker* pTicker);
		/*
		@brief: ��ȡ���ӹ�����
		*/
		CBaseConnectionMgr*			getBaseConnectionMgr() const;
		/*
		@brief: ���ݷ���ID��ȡ����
		*/
		CServiceBase*				getServiceBase(uint16_t nServiceID) const;
		/**
		@brief: ��ȡ���ڵ������Ϣ
		*/
		const SNodeBaseInfo&		getNodeBaseInfo() const;
		/**
		@brief: ��ȡ���ڵ������Ϣ
		*/
		uint16_t					getNodeID() const;
		/**
		@brief: ���ݽڵ����ֻ�ȡ�ڵ�id
		*/
		uint16_t					getServiceID(const std::string& szName) const;
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&			getConfigFileName() const;
		/*
		@brief: ��ȡQPS
		*/
		uint32_t					getQPS() const;
		/*
		@brief: �����Ƿ����������Ϣ
		*/
		void						debugLog(bool bEnable);
		/**
		@brief: �����Ƿ�����ܷ���
		*/
		void						profiling(bool bEnable);
		/**
		@brief: �����˳�
		*/
		void						doQuit();
	};
}