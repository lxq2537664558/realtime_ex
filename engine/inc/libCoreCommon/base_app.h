#pragma once

#include "core_common.h"
#include "service_base.h"
#include "actor_id_converter.h"
#include "ticker.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	class CBaseConnectionMgr;
	class CLogicRunnable;
	class CCoreApp;
	/**
	@brief: ���������
	*/
	class __CORE_COMMON_API__ CBaseApp
	{
		friend class CCoreApp;
		friend class CLogicRunnable;
		
	public:
		CBaseApp();
		virtual ~CBaseApp();

		static CBaseApp*& Inst();

		/**
		@brief: �������
		*/
		bool						run(const std::string& szInstanceName, const std::string& szConfig);
		/**
		@brief: ע�ᶨʱ��
		nStartTime ��һ�δ�����ʱ����ʱ��
		nIntervalTime ��һ�δ�����ʱ�����������ʱ�������ļ��ʱ�䣬�����ֵ��0�ͱ�ʾ�����ʱ��ֻ����һ��
		*/
		void						registerTicker(uint8_t nType, uint32_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void						unregisterTicker(CTicker* pTicker);
		/*
		@brief: ��ȡ���ӹ�����
		*/
		CBaseConnectionMgr*			getBaseConnectionMgr() const;
		/**
		@brief: ��ӷ���֮���ǰ�ù�����
		*/
		void						addGlobalBeforeFilter(const std::string& szKey, const NodeGlobalFilter& callback);
		/**
		@brief: ɾ������֮���ǰ�ù�����
		*/
		void						delGlobalBeforeFilter(const std::string& szKey);
		/**
		@brief: ��ӷ���֮��ĺ��ù�����
		*/
		void						addGlobalAfterFilter(const std::string& szKey, const NodeGlobalFilter& callback);
		/**
		@brief: ɾ������֮��ĺ��ù�����
		*/
		void						delGlobalAfterFilter(const std::string& szKey);
		/*
		@brief: ���ݷ���ID��ȡ����
		*/
		CServiceBase*				getServiceBase(uint32_t nServiceID) const;
		/**
		@brief: ��ȡ���ڵ������Ϣ
		*/
		const SNodeBaseInfo&		getNodeBaseInfo() const;
		/**
		@brief: ��ȡ���ڵ������Ϣ
		*/
		uint32_t					getNodeID() const;
		/**
		@brief: ���ݽڵ����ֻ�ȡ�ڵ�id
		*/
		uint32_t					getServiceID(const std::string& szName) const;
		/*
		@brief: ��ȡ�����ļ���
		*/
		const char*					getConfigFileName() const;
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

	protected:
		virtual bool				onInit() { return true; }
		virtual bool				onProcess() { return true; }
		virtual void				onDestroy() { }
	};
}