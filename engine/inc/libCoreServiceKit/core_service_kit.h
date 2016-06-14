#pragma once
#include "libBaseCommon/singleton.h"
#include "libBaseLua/lua_facade.h"
#include "core_service_kit_define.h"

namespace core
{
	class CCoreServiceKit :
		public base::CSingleton<CCoreServiceKit>
	{
	public:
		CCoreServiceKit();
		~CCoreServiceKit();

		bool							init();
		virtual void					release();

		/**
		@brief: 获取本服务基本信息
		*/
		const SServiceBaseInfo&			getServiceBaseInfo() const;
		/**
		@brief: 根据消息ID获取消息名字
		*/
		const std::string&				getMessageName(uint32_t nMessageID) const;
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
	};
}