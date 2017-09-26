#pragma once

#include <functional>
#include <vector>
#include <string>
#include <list>
#include <memory>

#include "ticker.h"
#include "core_common.h"

namespace core
{
	struct SPendingResponseInfo
	{
		CTicker		tickTimeout;
		uint64_t	nSessionID;
		uint64_t	nCoroutineID;
		uint64_t	nHolderID;
		std::list<uint64_t>::iterator
					iterHolder;

		int64_t		nBeginTime;	// ∑¢ÀÕ ±º‰
		uint32_t	nToServiceID;
		std::function<void(std::shared_ptr<void>, uint32_t)>
					callback;
	};
}