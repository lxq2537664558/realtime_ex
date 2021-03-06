#pragma once

#include <stdint.h>
#include <functional>

#include "core_common.h"

namespace core
{
	namespace coroutine
	{

		enum ECoroutineState
		{
			eCS_NONE,
			eCS_DEAD,	// 死亡状态
			eCS_READY,	// 等待执行（重来都没有被执行过）
			eCS_RUNNING,// 执行状态
			eCS_SUSPEND,// 挂起状态
		};

		__CORE_COMMON_API__ void		init();
		/**
		@brief: 启动协程，传入协程入口函数
		如果传入的栈大小是0的话就用共享栈，否则就用独立栈
		*/
		__CORE_COMMON_API__ uint64_t	create(uint32_t nStackSize, const std::function<void(uint64_t)>& fn);
		/**
		@brief: 关闭指定协程
		*/
		__CORE_COMMON_API__ void		close(uint64_t nID);
		/**
		@brief: 恢复某一个协程执行，可以传一个参数，yield函数返回后会取到，如果协程第一次执行就是入口函数的参数
		*/
		__CORE_COMMON_API__ void		resume(uint64_t nID, uint64_t nContext);
		/**
		@brief: 挂起当前执行的协程，在协程下次会执行时会返回resume函数传给他的参数
		*/
		__CORE_COMMON_API__ uint64_t	yield();
		/**
		@brief: 获取协程状态
		*/
		__CORE_COMMON_API__ uint32_t	getState(uint64_t nID);
		/**
		@brief: 设置协程数据
		*/
		__CORE_COMMON_API__ void		setLocalData(uint64_t nID, const char* szName, uint64_t nData);
		/**
		@brief: 获取协程数据
		*/
		__CORE_COMMON_API__ bool		getLocalData(uint64_t nID, const char* szName, uint64_t& nData);
		/**
		@brief: 删除协程数据
		*/
		__CORE_COMMON_API__ void		delLocalData(uint64_t nID, const char* szName);
		/**
		@brief: 获取当前执行协程的ID
		*/
		__CORE_COMMON_API__ uint64_t	getCurrentID();
		/**
		@brief: 获取协程数量
		*/
		__CORE_COMMON_API__ uint32_t	getCoroutineCount();
		/**
		@brief: 获取总的独立栈大小
		*/
		__CORE_COMMON_API__ uint64_t	getTotalStackSize();
	}
}