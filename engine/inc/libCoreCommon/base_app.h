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
	class CCoreConnectionMgr;
	class CCoreApp;
	/**
	@brief: 基础框架类
	*/
	class CBaseApp
	{
		friend class CCoreApp;
		friend class CCoreConnectionMgr;

	public:
		CBaseApp();
		virtual ~CBaseApp();

		static CBaseApp*& Inst();

		/**
		@brief: 启动框架
		*/
		bool						run(int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void						registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void						unregisterTicker(CTicker* pTicker);
		/**
		@brief: 获取当前逻辑时间
		*/
		int64_t						getLogicTime() const;
		/*
		@brief: 获取连接管理器
		*/
		CBaseConnectionMgr*			getBaseConnectionMgr() const;
		/*
		@brief: 获取配置文件名
		*/
		const std::string&			getConfigFileName() const;
		/*
		@brief: 获取写buf对象，的主要用于消息打包
		*/
		base::CWriteBuf&			getWriteBuf() const;
		/*
		@brief: 获取QPS
		*/
		uint32_t					getQPS() const;
		/*
		@brief: 标记繁忙，这样逻辑线程可以不等待消息队列的数据
		*/
		void						busy();
		/*
		@brief: 设置是否输出调试信息
		*/
		void						debugLog(bool bEnable);
		/**
		@brief: 设置是否打开性能分析
		*/
		void						profiling(bool bEnable);
		
	protected:
		virtual bool				onInit() { return true; }
		virtual void				onProcess() { }
		virtual void				onDestroy() { }

		virtual	void				onQuit() = 0;
		void						doQuit();
	};
}