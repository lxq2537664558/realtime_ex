#pragma once

#include "core_common.h"
#include "service_base.h"
#include "actor_id_converter.h"
#include "ticker.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	class CBaseConnectionMgr;
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
		bool						run(int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void						registerTicker(uint32_t nType, uint64_t nFrom, CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void						unregisterTicker(CTicker* pTicker);
		/**
		@brief: 注册普通的服务回调
		*/
		void						registerCallback(uint16_t nServiceID, const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback);
		/**
		@brief: 注册网关服务过来的服务回调
		*/
		void						registerGateForwardCallback(uint16_t nServiceID, const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback);

		/*
		@brief: 获取连接管理器
		*/
		CBaseConnectionMgr*			getBaseConnectionMgr() const;
		/*
		@brief: 根据服务ID获取服务
		*/
		CServiceBase*				getServiceBase(uint16_t nServiceID) const;
		/**
		@brief: 获取服务列表
		*/
		const std::vector<CServiceBase*>
									getServiceBase() const;
		/**
		@brief: 获取本节点基本信息
		*/
		const SNodeBaseInfo&		getNodeBaseInfo() const;
		/**
		@brief: 根据节点名字获取节点id
		*/
		uint16_t					getServiceID(const std::string& szName) const;
		/**
		@brief: 设置全局的服务连接成功回调
		*/
		void						setServiceConnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: 设置全局的服务连接断开回调
		*/
		void						setServiceDisconnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: 设置actorid转换器
		*/
		void						setActorIDConverter(CActorIDConverter* pActorIDConverter);
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
	};
}