#pragma once

#include "tinyxml2/tinyxml2.h"

#include "core_common.h"
#include "ticker.h"
#include "base_connection_mgr.h"

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

	class CCoreConnectionMgr;
	class CTickerMgr;
	class CMessagePort;
	/**
	@brief: 基础框架类，单例，主要管理游戏中各个管理器
	*/
	class CCoreApp
	{
	public:
		CCoreApp();
		virtual ~CCoreApp();

		static CCoreApp*& Inst();

		/**
		@brief: 启动框架
		*/
		bool					run(int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: 获取服务基本信息
		*/
		const SServiceBaseInfo&	getServiceBaseInfo() const;
		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void					registTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void					unregistTicker(CTicker* pTicker);
		/**
		@brief: 获取当前逻辑时间
		*/
		int64_t					getLogicTime() const;
		/*
		@brief: 获取连接管理器
		*/
		CBaseConnectionMgr*		getBaseConnectionMgr() const;
		/*
		@brief: 获取消息邮局
		*/
		CMessagePort*			getMessagePort() const;
		
	protected:
		virtual bool			onInit();
		virtual bool			onProcess();
		virtual void			onDestroy();

		virtual void			onBeforeFrame() { }
		virtual void			onAfterFrame() { }

		virtual	void			onQuit() = 0;
		void					doQuit();

	private:
		void					onAnalyze();

	protected:
		tinyxml2::XMLElement*		m_pRootXML;
		CTickerMgr*					m_pTickerMgr;
		CCoreConnectionMgr*			m_pCoreConnectionMgr;
		CMessagePort*				m_pMessagePort;
		SServiceBaseInfo			m_sServiceBaseInfo;
		uint32_t					m_nCycleCount;
		uint32_t					m_nTotalTime;
		volatile uint32_t			m_nRunState;
		bool						m_bMarkQuit;	// 这个参数作用是只触发一次onQuit
	};
}