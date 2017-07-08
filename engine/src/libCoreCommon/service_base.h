#pragma once

#include "core_common.h"
#include "ticker.h"

#include "libBaseCommon/buf_file.h"

namespace core
{
	/**
	@brief: 服务基础类
	*/
	class CServiceBase
	{
	public:
		CServiceBase();
		virtual ~CServiceBase();

		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: 反注册定时器
		*/
		void				unregisterTicker(CTicker* pTicker);
		/*
		@brief: 获取配置文件名
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: 获取写buf对象，的主要用于消息打包
		*/
		base::CWriteBuf&	getWriteBuf() const;
		/*
		@brief: 获取QPS
		*/
		uint32_t			getQPS() const;

	protected:
		virtual bool		onInit() { return true; }
		virtual void		onProcess() { }
		virtual void		onDestroy() { }
	};
}