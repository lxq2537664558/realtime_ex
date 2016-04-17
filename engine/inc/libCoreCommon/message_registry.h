#pragma once
#include "libBaseCommon/singleton.h"

#include "core_common.h"

#include <map>

namespace core
{
	/**
	@brief: 消息响应注册器
	*/
	class CMessageRegistry :
		public base::CSingleton<CMessageRegistry>
	{
	public:
		CMessageRegistry();
		~CMessageRegistry();

		bool init();

		/**
		@brief: 注册普通的服务响应函数
		*/
		void registCallback(uint32_t nMessageID, ServiceCallback callback);
		/**
		@brief: 注册经网关服务转发客户端消息响应函数
		*/
		void registCallback(uint32_t nMessageID, GateClientCallback callback);
		/**
		@brief: 注册服务之间的前置过滤器
		*/
		void registGlobalBeforeFilter(ServiceGlobalFilter callback);
		/**
		@brief: 注册服务之间的后置过滤器
		*/
		void registGlobalAfterFilter(ServiceGlobalFilter callback);
	};
}