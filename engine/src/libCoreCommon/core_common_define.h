#pragma once

#include <functional>
#include <vector>
#include <string>

#include "ticker.h"
#include "core_common.h"

#include "google\protobuf\message.h"

namespace core
{
	struct SPendingResponseInfo
	{
		CTicker		tickTimeout;
		uint64_t	nSessionID;
		uint64_t	nCoroutineID;

		uint64_t	nToID;		// 目标ID
		int64_t		nBeginTime;	// 发送时间
		std::string	szMessageName;	// 消息ID
		
		std::function<void(std::shared_ptr<google::protobuf::Message>, uint32_t)>
					callback;
	};
}