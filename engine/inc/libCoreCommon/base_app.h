#pragma once

#include "core_common.h"
#include "ticker.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	// 服务器状态 eARS_Start->eARS_Normal->eARS_Quitting->eARS_Quit
	// 其中eARS_Quitting状态切换到eARS_Quit是有逻辑层主动调用doQuit来完成的，
	// 这样做保证了关服前做一些需要比较长的时间来确认一些事的行为，比如数据存储
	enum EAppRunState
	{
		eARS_Start		= 0,	// 启动状态
		eARS_Normal		= 1,	// 正常状态
		eARS_Quitting	= 2,	// 退出中
		eARS_Quit		= 3,	// 最终退出
	};

	class CBaseConnectionMgr;
	class ILoadBalancePolicy;
	class CCoreApp;
	/**
	@brief: 基础框架类
	*/
	class CBaseApp
	{
		friend class CCoreApp;

	public:
		CBaseApp();
		virtual ~CBaseApp();

		static CBaseApp*& Inst();

		/**
		@brief: 启动框架
		*/
		bool	run(bool bNormalService, int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: 获取本服务基本信息
		*/
		const SServiceBaseInfo&	
				getServiceBaseInfo() const;
		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void	registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void	unregisterTicker(CTicker* pTicker);
		/**
		@brief: 获取当前逻辑时间
		*/
		int64_t	getLogicTime() const;
		/*
		@brief: 获取连接管理器
		*/
		CBaseConnectionMgr*	
				getBaseConnectionMgr() const;
		/*
		@brief: 获取配置文件名
		*/
		const std::string&		
				getConfigFileName() const;
		/*
		@brief: 获取写buf对象，的主要用于消息打包
		*/
		base::CWriteBuf&		
				getWriteBuf() const;
		/*
		@brief: 注册负载均衡器
		*/
		void	registLoadBalancePolicy(ILoadBalancePolicy* pLoadBalancePolicy);
		/*
		@brief: 获取负载均衡器
		*/
		ILoadBalancePolicy*	
				getLoadBalancePolicy(uint32_t nID) const;
		/*
		@brief: 根据消息名字获取所有支持该消息的服务名字
		*/
		const std::vector<std::string>&
				getServiceName(const std::string& szMessageName) const;
		/*
		@brief: 根据消息ID获取消息名字
		*/
		const std::string&
				getMessageName(uint32_t nMessageID) const;
		
	protected:
		virtual bool	onInit() { return true; }
		virtual void	onProcess() { }
		virtual void	onDestroy() { }

		virtual void	onBeforeFrame() { }
		virtual void	onAfterFrame() { }

		virtual	void	onQuit() = 0;
		void			doQuit();
	};
}