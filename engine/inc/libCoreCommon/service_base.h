#pragma once

#include "core_common.h"
#include "ticker.h"
#include "service_invoker.h"
#include "base_object.h"
#include "actor_id_converter.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	// 服务状态 eSRS_Start->eSRS_Normal->eSRS_Quitting->eSRS_Quit
	// 其中eSRS_Quitting状态切换到eSRS_Quit是有逻辑层主动调用doQuit来完成的，
	// 这样做保证了关服前做一些需要比较长的时间来确认一些事的行为，比如数据存储
	enum EServiceRunState
	{
		eSRS_Start = 0,		// 启动状态
		eSRS_Normal = 1,	// 正常状态
		eSRS_Quitting = 2,	// 退出中
		eSRS_Quit = 3,		// 最终退出
	};

	class CActorBase;
	class CServiceBaseImpl;
	/**
	@brief: 服务基础类
	*/
	class CServiceBase :
		public CBaseObject
	{
		friend class CServiceBaseImpl;
		
	public:
		CServiceBase();
		virtual ~CServiceBase();

		/**
		@brief: 获取服务ID
		*/
		const SServiceBaseInfo&	getServiceBaseInfo() const;

		/**
		@brief: 设置actorid转换器
		*/
		void					setActorIDConverter(CActorIDConverter* pActorIDConverter);
		/**
		@brief: 获取actorid转换器
		*/
		CActorIDConverter*		getActorIDConverter() const;

		/**
		@brief: 注册定时器
		nStartTime 第一次触发定时器的时间
		nIntervalTime 第一次触发定时器后接下来定时器触发的间隔时间，如果该值是0就表示这个定时器只触发一次
		*/
		void					registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void					unregisterTicker(CTicker* pTicker);

		/**
		@brief: 注册普通服务消息
		*/
		void					registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(SSessionInfo, google::protobuf::Message*)>& callback);
		/**
		@brief: 注册经网关服务转发客户端的服务消息
		*/
		void					registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(SClientSessionInfo, google::protobuf::Message*)>& callback);
		
		/**
		@brief: 注册普通actor消息
		*/
		void					registerActorMessageHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		/**
		@brief: 注册经网关服务转发客户端的actor消息
		*/
		void					registerActorForwardHandler(const std::string& szMessageName, const std::function<void(CActorBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);

		/**
		@brief: 设置全局的服务连接成功回调
		*/
		void					setServiceConnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: 设置全局的服务连接断开回调
		*/
		void					setServiceDisconnectCallback(const std::function<void(uint16_t)>& callback);

		/**
		@brief: 获取服务调用器
		*/
		CServiceInvoker*		getServiceInvoker() const;
		/**
		@brief: 创建actor
		*/
		CActorBase*				createActor(const std::string& szClassName, void* pContext);
		/*
		@brief: 获取配置文件名
		*/
		const std::string&		getConfigFileName() const;
		/*
		@brief: 获取写buf对象，的主要用于消息打包
		*/
		base::CWriteBuf&		getWriteBuf() const;
		/*
		@brief: 获取QPS
		*/
		uint32_t				getQPS() const;
		/*
		@brief: 获取运行状态
		*/
		EServiceRunState		getState() const;
		/*
		@brief: 逻辑发出退出
		*/
		void					doQuit();

	protected:
		virtual bool			onInit() = 0;
		virtual void			onFrame() { }
		virtual void			onQuit() = 0;

	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}