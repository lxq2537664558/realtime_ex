#pragma once

#include "core_common.h"
#include "ticker.h"
#include "service_invoker.h"
#include "base_object.h"
#include "actor_id_converter.h"
#include "service_id_converter.h"
#include "protobuf_factory.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	// 服务状态 eSRS_Start->eSRS_Normal->eSRS_Quitting->eSRS_Quit
	// 其中eSRS_Quitting状态切换到eSRS_Quit是有逻辑层主动调用doQuit来完成的，
	// 这样做保证了关服前做一些需要比较长的时间来确认一些事的行为，比如数据存储
	enum EServiceRunState
	{
		eSRS_Start		= 0,	// 启动状态
		eSRS_Normal		= 1,	// 正常状态
		eSRS_Quitting	= 2,	// 退出中
		eSRS_Quit		= 3,	// 最终退出
	};

	class CActorBase;
	class CServiceBaseImpl;
	/**
	@brief: 服务基础类
	*/
	class __CORE_COMMON_API__ CServiceBase :
		public CBaseObject
	{
		friend class CServiceBaseImpl;
		
	public:
		CServiceBase();
		virtual ~CServiceBase();

		uint32_t				getServiceID() const;
		/**
		@brief: 获取服务基础信息
		*/
		const SServiceBaseInfo&	getServiceBaseInfo() const;

		/**
		@brief: 设置actor_id转换器
		*/
		void					setActorIDConverter(CActorIDConverter* pActorIDConverter);
		/**
		@brief: 设置service_id转换器
		*/
		void					setServiceIDConverter(CServiceIDConverter* pServiceIDConverter);

		/**
		@brief: 设置protobuf工厂，目的是为了创建出真实的protibuf，而不是反射出来的
		*/
		void					setProtobufFactory(CProtobufFactory* pProtobufFactory);

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
		void					registerServiceMessageHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SSessionInfo, const google::protobuf::Message*)>& callback);
		/**
		@brief: 注册经网关服务转发客户端的服务消息
		*/
		void					registerServiceForwardHandler(const std::string& szMessageName, const std::function<void(CServiceBase*, SClientSessionInfo, const google::protobuf::Message*)>& callback);
		
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
		void					setServiceConnectCallback(const std::function<void(uint32_t)>& callback);
		/**
		@brief: 设置全局的服务连接断开回调
		*/
		void					setServiceDisconnectCallback(const std::function<void(uint32_t)>& callback);

		/**
		@brief: 获取服务调用器
		*/
		CServiceInvoker*		getServiceInvoker() const;
		/**
		@brief: 创建actor
		*/
		CActorBase*				createActor(const std::string& szClassName, uint64_t nActorID, const std::string& szContext);
		/*
		@brief: 获取运行状态
		*/
		EServiceRunState		getRunState() const;
		/*
		@brief: 逻辑发出退出
		*/
		void					doQuit();
		
		virtual void			release();

	protected:
		virtual bool			onInit() = 0;
		virtual void			onFrame() { }
		virtual void			onQuit() = 0;

	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}