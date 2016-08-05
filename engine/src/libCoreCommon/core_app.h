#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "ticker_mgr.h"
#include "coroutine_mgr.h"
#include "core_connection_mgr.h"
#include "logic_message_queue.h"

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
		CBaseConnectionMgr*	getBaseConnectionMgr() const;
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
		@brief: 获取采样时间
		*/
		uint32_t			getSamplingTime() const;
		/*
		@brief: 获取消息队列
		*/
		CLogicMessageQueue*	getMessageQueue() const;
		/*
		@brief: 获取QPS
		*/
		uint32_t			getQPS() const;
		/*
		@brief: 增加QPS
		*/
		void				incQPS();
		/*
		@brief: 标记繁忙，这样逻辑线程可以不等待消息队列的数据
		*/
		void				busy();

	private:
		bool				onInit();
		bool				onProcess();
		void				onDestroy();
		void				onQPS(uint64_t nContext);

	protected:
		std::string				m_szConfig;
		std::string				m_szPID;
		CTickerMgr*				m_pTickerMgr;
		CBaseConnectionMgr*		m_pBaseConnectionMgr;
		CCoroutineMgr*			m_pCoroutineMgr;
		CLogicMessageQueue*		m_pMessageQueue;
		base::CWriteBuf			m_writeBuf;
		uint32_t				m_nCycleCount;
		int64_t					m_nTotalSamplingTime;
		uint32_t				m_nSamplingTime;
		volatile uint32_t		m_nRunState;
		uint32_t				m_nHeartbeatLimit;
		uint32_t				m_nHeartbeatTime;
		bool					m_bMarkQuit;	// 这个参数作用是只触发一次onQuit
		uint32_t				m_nQPS;
		CTicker					m_tickerQPS;
		bool					m_bBusy;
	};
}