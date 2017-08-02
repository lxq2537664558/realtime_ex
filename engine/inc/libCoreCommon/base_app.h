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
	@brief: 基础框架类
	*/
	class __CORE_COMMON_API__ CBaseApp :
		public base::noncopyable
	{
		friend class CCoreApp;
		friend class CLogicRunnable;
		
	public:
		CBaseApp();
		virtual ~CBaseApp();

		static CBaseApp*& Inst();

		/**
		@brief: 启动框架
		*/
		bool						run(const std::string& szInstanceName, const std::string& szConfig);
		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void						registerTicker(uint8_t nType, uint32_t nFromServiceID, uint64_t nFromActorID, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void						unregisterTicker(CTicker* pTicker);
		/*
		@brief: 获取连接管理器
		*/
		CBaseConnectionMgr*			getBaseConnectionMgr() const;
		/**
		@brief: 添加服务之间的特定消息的前置过滤器（排除掉定时器，心跳，request，response，gate_forward）
		*/
		void						addGlobalBeforeFilter(uint8_t nMessageType, const std::string& szKey, const NodeGlobalFilter& callback);
		/**
		@brief: 删除服务之间的特定消息的前置过滤器（排除掉定时器，心跳，request，response，gate_forward）
		*/
		void						delGlobalBeforeFilter(uint8_t nMessageType, const std::string& szKey);
		/**
		@brief: 添加服务之间的特定消息的后置过滤器（排除掉定时器，心跳，request，response，gate_forward）
		*/
		void						addGlobalAfterFilter(uint8_t nMessageType, const std::string& szKey, const NodeGlobalFilter& callback);
		/**
		@brief: 删除服务之间的特定消息的后置过滤器（排除掉定时器，心跳，request，response，gate_forward）
		*/
		void						delGlobalAfterFilter(uint8_t nMessageType, const std::string& szKey);
		/*
		@brief: 根据服务ID获取服务
		*/
		CServiceBase*				getServiceBase(uint32_t nServiceID) const;
		/**
		@brief: 获取本节点基本信息
		*/
		const SNodeBaseInfo&		getNodeBaseInfo() const;
		/**
		@brief: 获取本节点基本信息
		*/
		uint32_t					getNodeID() const;
		/**
		@brief: 根据节点名字获取节点id
		*/
		uint32_t					getServiceID(const std::string& szName) const;
		/**
		@brief: 判断是否是本节点的服务
		*/
		bool						isLocalService(uint32_t nServiceID) const;
		/**
		@brief: 根据服务类型取到该类型服务的所有服务id
		*/
		const std::vector<uint32_t>&getServiceIDByTypeName(const std::string& szName) const;
		/*
		@brief: 获取配置文件名
		*/
		const std::string&			getConfigFileName() const;
		/*
		@brief: 获取QPS
		*/
		uint32_t					getQPS() const;
		/*
		@brief: 设置是否输出调试信息
		*/
		void						debugLog(bool bEnable);
		/**
		@brief: 设置是否打开性能分析
		*/
		void						profiling(bool bEnable);
		/**
		@brief: 发起退出
		*/
		void						doQuit();

	protected:
		virtual bool				onInit() { return true; }
		virtual bool				onProcess() { return true; }
		virtual void				onDestroy() { }
	};
}