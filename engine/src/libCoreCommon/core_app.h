#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "ticker_mgr.h"
#include "coroutine_mgr.h"
#include "core_connection_mgr.h"

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
		bool				run(int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void				unregisterTicker(CTicker* pTicker);
		/**
		@brief: 获取当前逻辑时间
		*/
		int64_t				getLogicTime() const;
		/*
		@brief: 获取连接管理器
		*/
		CCoreConnectionMgr*	getCoreConnectionMgr() const;
		/*
		@brief: 获取携程管理器
		*/
		CCoroutineMgr*		getCoroutineMgr() const;
		/*
		@brief: 获取配置文件名
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: 获取写buf对象，的主要用于消息打包
		*/
		base::CWriteBuf&	getWriteBuf() const;
		/*
		@brief: 退出框架
		*/
		void				doQuit();
		/*
		@brief: 获取连接心跳超时极限次数
		*/
		uint32_t			getHeartbeatLimit() const;
		/*
		@brief: 获取连接心跳同步时间
		*/
		uint32_t			getHeartbeatTime() const;
		/*
		@brief: 获取QPS
		*/
		uint32_t			getQPS() const;
		/*
		@brief: 增加QPS
		*/
		void				incQPS();

	private:
		bool				onInit();
		bool				onProcess();
		void				onDestroy();
		void				onAnalyze();
		void				onQPS(uint64_t nContext);

	protected:
		std::string				m_szConfig;
		std::string				m_szPID;
		CTickerMgr*				m_pTickerMgr;
		CCoreConnectionMgr*		m_pCoreConnectionMgr;
		CCoroutineMgr*			m_pCoroutineMgr;
		base::CWriteBuf			m_writeBuf;
		uint32_t				m_nCycleCount;
		uint32_t				m_nTotalTime;
		uint32_t				m_nSamplingTime;
		volatile uint32_t		m_nRunState;
		uint32_t				m_nHeartbeatLimit;
		uint32_t				m_nHeartbeatTime;
		bool					m_bMarkQuit;	// 这个参数作用是只触发一次onQuit
		uint32_t				m_nQPS;
		CTicker					m_tickQPS;
	};
}