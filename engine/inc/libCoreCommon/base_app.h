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
	@brief: 基础框架类
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
		@brief: 启动框架
		*/
		bool				runAndServe(size_t argc, char** argv, const std::vector<CServiceBase*>& vecServiceBase);
		/*
		@brief: 根据服务ID获取服务
		*/
		CServiceBase*		getServiceBase(uint32_t nServiceID) const;
		/**
		@brief: 获取本节点基本信息
		*/
		const SNodeBaseInfo&
							getNodeBaseInfo() const;
		/**
		@brief: 获取本节点基本信息
		*/
		uint32_t			getNodeID() const;
		/**
		@brief: 判断是否是本节点的服务
		*/
		bool				isLocalService(uint32_t nServiceID) const;
		/*
		@brief: 获取配置文件名
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: 获取协程调用栈大小
		*/
		uint32_t			getCoroutineStackSize() const;
		/*
		@brief: 获取QPS
		*/
		uint32_t			getQPS() const;
		/*
		@brief: 设置是否输出调试信息
		*/
		void				debugLog(bool bEnable);
		/**
		@brief: 设置是否打开性能分析
		*/
		void				profiling(bool bEnable);
		/**
		@brief: 发起退出
		*/
		void				doQuit();
	};
}