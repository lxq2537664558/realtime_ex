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
		@brief: 获取本节点基本信息
		*/
		const SNodeBaseInfo&			getNodeBaseInfo() const;
		/**
		@brief: 根据节点名字获取节点id
		*/
		uint16_t						getNodeID(const std::string& szName) const;
		/**
		@brief: 设置全局的节点连接成功回调
		*/
		void							setNodeConnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: 设置全局的节点连接断开回调
		*/
		void							setNodeDisconnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: 注册普通节点消息
		*/
		void							registerMessageHandler(uint16_t nMessageID, const std::function<bool(uint16_t, CMessage)>& callback);
		/**
		@brief: 注册经网关节点转发客户端的消息
		*/
		void							registerForwardHandler(uint16_t nMessageID, const std::function<bool(SClientSessionInfo, CMessage)>& callback);
		/**
		@brief: 添加节点之间的前置过滤器
		*/
		void							addGlobalBeforeFilter(GlobalBeforeFilter callback);
		/**
		@brief: 添加节点之间的后置过滤器
		*/
		void							addGlobalAfterFilter(GlobalAfterFilter callback);
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