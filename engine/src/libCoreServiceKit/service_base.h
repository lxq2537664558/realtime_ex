#pragma once

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

#include <functional>
#include <vector>

namespace core
{
	struct SRequestMessageInfo
	{
		uint64_t		nFromActorID;
		uint64_t		nToActorID;
		uint64_t		nSessionID;
		message_header*	pData;
	};

	struct SResponseMessageInfo
	{
		uint64_t		nFromActorID;
		uint64_t		nToActorID;
		uint64_t		nSessionID;
		uint8_t			nResult;
		message_header*	pData;
	};

	struct SGateForwardMessageInfo
	{
		uint64_t		nActorID;
		uint64_t		nSessionID;
		message_header*	pData;
	};

	struct SGateMessageInfo
	{
		uint64_t		nSessionID;
		message_header*	pData;
	};

	struct SGateBroadcastMessageInfo
	{
		std::vector<uint64_t>	vecSessionID;
		message_header*			pData;
	};

	struct SResponseWaitInfo
	{
		CTicker		tickTimeout;
		uint64_t	nSessionID;
		uint64_t	nCoroutineID;

		uint64_t	nTraceID;	// traceid
		uint64_t	nToID;		// 目标ID
		int64_t		nBeginTime;	// 发送时间
		uint16_t	nMessageID;	// 消息ID
		
		std::function<void(CMessage, uint32_t)>
					callback;
	};

	enum EBaseConnectionType
	{
		eBCT_ConnectionToMaster			= 10,
		eBCT_ConnectionFromOtherNode	= 11,
		eBCT_ConnectionToOtherNode		= 12,
	};

	struct STraceInfo
	{
		uint64_t	nTraceID;
		std::string	szParentServiceName;
	};
}