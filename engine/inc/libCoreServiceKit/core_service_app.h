#pragma once
#include "libCoreCommon/base_app.h"

#include "libBaseLua/lua_facade.h"

#include "core_service_kit_common.h"
#include "actor_id_converter.h"
#include "service_invoker.h"

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
		const SNodeBaseInfo&	getNodeBaseInfo() const;
		/**
		@brief: 根据节点名字获取节点id
		*/
		uint16_t				getServiceID(const std::string& szName) const;
		/**
		@brief: 设置全局的节点连接成功回调
		*/
		void					setNodeConnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: 设置全局的节点连接断开回调
		*/
		void					setNodeDisconnectCallback(const std::function<void(uint16_t)>& callback);
		/**
		@brief: 设置actorid转换器
		*/
		void					setActorIDConverter(CActorIDConverter* pActorIDConverter);
		/**
		@brief: 注册普通节点消息
		*/
		void					registerMessageHandler(uint16_t nServiceID, const std::string& szMessageName, const std::function<bool(uint16_t, google::protobuf::Message*)>& callback);
		/**
		@brief: 注册经网关节点转发客户端的消息
		*/
		void					registerForwardHandler(uint16_t nServiceID, const std::string& szMessageName, const std::function<bool(SClientSessionInfo, google::protobuf::Message*)>& callback);
		/**
		@brief: 获取lua包装对象
		*/
		base::CLuaFacade*		getLuaFacade() const;
		/**
		@brief: 获取某一个服务的调用器
		*/
		CServiceInvoker*		getServiceInvoker(uint16_t nServiceID) const;
		
	protected:
		virtual bool			onInit();
		virtual void			onProcess();
		virtual void			onDestroy();
	};

}