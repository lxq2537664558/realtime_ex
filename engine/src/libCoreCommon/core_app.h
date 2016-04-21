#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "ticker_mgr.h"
#include "core_connection_mgr.h"
#include "service_mgr.h"
#include "transport.h"
#include "message_directory.h"
#include "load_balance_policy_mgr.h"

namespace core
{
	/**
	@brief: 核心框架类，单例，主要管理游戏中各个管理器
	*/
	class CCoreApp :
		public base::CSingleton<CCoreApp>
	{
	public:
		CCoreApp();
		~CCoreApp();

		/**
		@brief: 启动框架
		*/
		bool	run(bool bNormalService, int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: 获取服务基本信息
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
		CCoreConnectionMgr*		
				getCoreConnectionMgr() const;
		/*
		@brief: 获取服务管理器
		*/
		CServiceMgr*			
				getServiceMgr() const;
		/*
		@brief: 获取消息传送器
		*/
		CTransport*			
				getTransport() const;
		/*
		@brief: 获取消息字典
		*/
		CMessageDirectory*		
				getMessageDirectory() const;
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
		void	registLoadBalancePolicy(ILoadBalancePolicy* pLoadBalance);
		/*
		@brief: 获取负载均衡器
		*/
		ILoadBalancePolicy*		
				getLoadBalancePolicy(uint32_t nID) const;
		/*
		@brief: 根据消息ID获取所有支持该消息的服务名字
		*/
		const std::vector<std::string>&
				getServiceName(const std::string& szMessageName) const;
		/*
		@brief: 退出框架
		*/
		void					doQuit();

	private:
		bool					onInit();
		bool					onProcess();
		void					onDestroy();
		void					onAnalyze();

	protected:
		std::string				m_szConfig;
		std::string				m_szPID;
		CTickerMgr*				m_pTickerMgr;
		CCoreConnectionMgr*		m_pCoreConnectionMgr;
		CServiceMgr*			m_pServiceMgr;
		CTransport*				m_pTransport;
		CMessageDirectory*		m_pMessageDirectory;
		CLoadBalancePolicyMgr*	m_pLoadBalancePolicyMgr;
		SServiceBaseInfo		m_sServiceBaseInfo;
		base::CWriteBuf			m_writeBuf;
		uint32_t				m_nCycleCount;
		uint32_t				m_nTotalTime;
		volatile uint32_t		m_nRunState;
		bool					m_bNormalService;
		bool					m_bMarkQuit;	// 这个参数作用是只触发一次onQuit
	};
}