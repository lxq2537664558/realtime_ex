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
		@brief: 设置全局的服务连接成功回调
		*/
		void							setServiceConnectCallback(std::function<void(uint16_t)> funConnect);
		/**
		@brief: 设置全局的服务连接断开回调
		*/
		void							setServiceDisconnectCallback(std::function<void(uint16_t)> funDisconnect);
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