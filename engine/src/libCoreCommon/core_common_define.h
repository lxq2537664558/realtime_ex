#pragma once

#include <functional>
#include <vector>
#include <string>

#include "core_common.h"

#include "google\protobuf\message.h"

namespace core
{
	struct SResponseWaitInfo
	{
		CTicker		tickTimeout;
		uint64_t	nSessionID;
		uint64_t	nCoroutineID;

		uint64_t	nToID;		// 目标ID
		int64_t		nBeginTime;	// 发送时间
		std::string	szMessageName;	// 消息ID
		
		std::function<void(google::protobuf::Message*, uint32_t)>
					callback;
	};

	enum EBaseConnectionType
	{
		eBCT_ConnectionToMaster			= 10,
		eBCT_ConnectionFromOtherNode	= 11,
		eBCT_ConnectionToOtherNode		= 12,
	};
}