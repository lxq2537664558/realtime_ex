#pragma once

#include "libCoreCommon/core_common.h"

#include "core_service_kit_common.h"

#include <functional>
#include <vector>

namespace core
{
	struct SRequestMessageInfo
	{
		uint64_t	nFromActorID;
		uint64_t	nToActorID;
		uint64_t	nSessionID;
		const google::protobuf::Message*
					pMessage;
	};

	struct SResponseMessageInfo
	{
		uint64_t	nFromActorID;
		uint64_t	nToActorID;
		uint64_t	nSessionID;
		uint8_t		nResult;
		const google::protobuf::Message*
					pMessage;
	};

	struct SGateForwardMessageInfo
	{
		uint64_t	nActorID;
		uint64_t	nSessionID;
		const google::protobuf::Message*
					pMessage;
	};

	struct SGateMessageInfo
	{
		uint64_t	nSessionID;
		const google::protobuf::Message*
					pMessage;
	};

	struct SGateBroadcastMessageInfo
	{
		std::vector<uint64_t>	vecSessionID;
		const google::protobuf::Message*
								pMessage;
	};

	struct SResponseWaitInfo
	{
		CTicker		tickTimeout;
		uint64_t	nSessionID;
		uint64_t	nCoroutineID;

		uint64_t	nToID;		// 目标ID
		int64_t		nBeginTime;	// 发送时间
		uint16_t	nMessageID;	// 消息ID
		
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