#pragma once
#include "libBaseCommon/singleton.h"
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
		@brief: 获取某一个消息的调用信息
		*/
		const SMessageProxyGroupInfo*	getMessageProxyGroupInfo(const std::string& szMessageName) const;
	};
}