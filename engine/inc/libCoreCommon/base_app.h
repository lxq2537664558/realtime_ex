#pragma once

#include "core_common.h"
#include "service_base.h"

#include "libBaseCommon/buf_file.h"

#include <set>

namespace core
{
	class CLogicRunnable;
	class CCoreApp;
	/**
	@brief: ���������
	*/
	class __CORE_COMMON_API__ CBaseApp :
		public base::noncopyable
	{
		friend class CCoreApp;
		friend class CLogicRunnable;
		
	public:
		CBaseApp();
		~CBaseApp();

		static CBaseApp*& Inst();

		/**
		@brief: �������
		*/
		bool				runAndServe(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		/*
		@brief: ���ݷ���ID��ȡ����
		*/
		CServiceBase*		getServiceBase(uint32_t nServiceID) const;
		/**
		@brief: ��ȡ���ڵ������Ϣ
		*/
		const SNodeBaseInfo&
							getNodeBaseInfo() const;
		/**
		@brief: ��ȡ���ڵ������Ϣ
		*/
		uint32_t			getNodeID() const;
		/**
		@brief: �ж��Ƿ��Ǳ��ڵ�ķ���
		*/
		bool				isLocalService(uint32_t nServiceID) const;
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: ��ȡЭ�̵���ջ��С
		*/
		uint32_t			getCoroutineStackSize() const;
		/*
		@brief: ��ȡQPS
		*/
		uint32_t			getQPS() const;
		/*
		@brief: �����Ƿ����������Ϣ
		*/
		void				debugLog(bool bEnable);
		/**
		@brief: �����Ƿ�����ܷ���
		*/
		void				profiling(bool bEnable);
		/**
		@brief: �����˳�
		*/
		void				doQuit();
	};
}