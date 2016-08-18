#pragma once
#include "libCoreCommon/base_app.h"

#include "libBaseLua/lua_facade.h"

#include "core_service_kit_define.h"

namespace core
{
	class CCoreServiceApp :
		public CBaseApp
	{
	public:
		CCoreServiceApp();
		~CCoreServiceApp();

		static CCoreServiceApp* Inst();

		/**
		@brief: 获取本服务基本信息
		*/
		const SServiceBaseInfo&			getServiceBaseInfo() const;
		/**
		@brief: 根据服务名字获取服务id
		*/
		uint16_t						getServiceID(const std::string& szServiceName) const;
		/**
		@brief: 设置全局的服务连接成功回调
		*/
		void							setServiceConnectCallback(std::function<void(uint16_t)> funConnect);
		/**
		@brief: 设置全局的服务连接断开回调
		*/
		void							setServiceDisconnectCallback(std::function<void(uint16_t)> funDisconnect);
		/**
		@brief: 注册普通服务消息
		*/
		void							registerServiceCallback(uint16_t nMessageID, ServiceCallback callback);
		/**
		@brief: 注册经网关服务转发客户端的消息
		*/
		void							registerGateForwardCallback(uint16_t nMessageID, GateForwardCallback callback);
		/**
		@brief: 添加服务之间的前置过滤器
		*/
		void							addGlobalBeforeFilter(ServiceGlobalFilter callback);
		/**
		@brief: 添加服务之间的后置过滤器
		*/
		void							addGlobalAfterFilter(ServiceGlobalFilter callback);
		/**
		@brief: 开始一个新的trace
		*/
		void							startNewTrace();
		/**
		@brief: 添加trace额外信息
		*/
		void							addTraceExtraInfo(const char* szFormat, ...);
		/**
		@brief: 获取lua包装对象
		*/
		base::CLuaFacade*				getLuaFacade() const;

	protected:
		virtual bool					onInit();
		virtual void					onProcess();
		virtual void					onDestroy();
	};

}