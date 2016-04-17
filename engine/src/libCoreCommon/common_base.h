#pragma once

#include "core_common.h"

#include <functional>
#include <vector>

namespace core
{
	struct SRequestMessageInfo
	{
		uint16_t				nMessageFormat;
		const message_header*	pData;
		InvokeCallback			callback;
	};

	struct SResponseMessageInfo
	{
		uint16_t				nMessageFormat;
		uint64_t				nSessionID;
		const message_header*	pData;
		uint8_t					nResult;
	};

	struct SGateMessageInfo
	{
		uint64_t				nSessionID;
		uint16_t				nMessageFormat;
		const message_header*	pData;
	};

	struct SGateBroadcastMessageInfo
	{
		std::vector<uint64_t>	vecSessionID;
		uint16_t				nMessageFormat;
		const message_header*	pData;
	};

	struct SMessageCacheInfo
	{
		uint32_t								nTotalSize;
		bool									bRefuse;
		std::vector<SRequestMessageInfo>		vecRequestMessageInfo;
		std::vector<SGateMessageInfo>			vecGateMessageInfo;
		std::vector<SGateBroadcastMessageInfo>	vecGateBroadcastMessageInfo;
	};

	struct SResponseWaitInfo
	{
		CTicker			tickTimeout;
		uint64_t		nSessionID;
		std::string		szServiceName;
		InvokeCallback	callback;
	};
}