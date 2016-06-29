#pragma once
#include "libBaseCommon/base_common.h"

#include <functional>

namespace core
{
	enum ECoroutineState
	{
		eCS_DEAD,
		eCS_READY,
		eCS_RUNNING,
		eCS_SUSPEND,
	};

	namespace coroutine
	{
		uint64_t	start(std::function<void(uint64_t)> fn);
		void		resume(uint64_t nID, uint64_t nContext);
		uint64_t	yield();
		void		sleep(int64_t ms);
		uint32_t	getState(uint64_t nID);
		void		sendMessage(uint64_t nID, void* pData);
		void*		recvMessage(uint64_t nID);
		uint64_t	getCurrentID();
	};
}