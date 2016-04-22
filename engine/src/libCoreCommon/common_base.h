#pragma once

#include "core_common.h"

#include <functional>
#include <vector>

namespace core
{
	struct SRequestMessageInfo
	{
		google::protobuf::Message*	pMessage;
		InvokeCallback				callback;
	};

	struct SResponseMessageInfo
	{
		uint64_t					nSessionID;
		google::protobuf::Message*	pMessage;
		uint8_t						nResult;
	};

	struct SGateForwardMessageInfo
	{
		uint64_t		nSessionID;
		message_header*	pHeader;
	};

	struct SGateMessageInfo
	{
		uint64_t					nSessionID;
		google::protobuf::Message*	pMessage;
	};

	struct SGateBroadcastMessageInfo
	{
		std::vector<uint64_t>		vecSessionID;
		google::protobuf::Message*	pMessage;
	};

	struct SRequestMessageCacheInfo
	{
		std::vector<char>	vecBuf;
		InvokeCallback		callback;
	};

	struct SGateMessageCacheInfo
	{
		uint64_t			nSessionID;
		std::vector<char>	vecBuf;
	};

	struct SGateForwardMessageCacheInfo
	{
		uint64_t			nSessionID;
		std::vector<char>	vecBuf;
	};

	struct SGateBroadcastMessageCacheInfo
	{
		std::vector<uint64_t>	vecSessionID;
		std::vector<char>		vecBuf;
	};

	struct SMessageCacheInfo
	{
		uint32_t										nTotalSize;
		bool											bRefuse;
		std::vector<SRequestMessageCacheInfo*>			vecRequestMessageCacheInfo;
		std::vector<SGateMessageCacheInfo*>				vecGateMessageCacheInfo;
		std::vector<SGateForwardMessageCacheInfo*>		vecGateForwardMessageCacheInfo;
		std::vector<SGateBroadcastMessageCacheInfo*>	vecGateBroadcastMessageCacheInfo;
	};

	struct SResponseWaitInfo
	{
		CTicker			tickTimeout;
		uint64_t		nSessionID;
		std::string		szServiceName;
		InvokeCallback	callback;
	};

	struct SServiceCallbackInfo
	{
		std::string		szMessageName;
		ServiceCallback	serviceCallback;
	};

	struct SGateClientCallbackInfo
	{
		std::string			szMessageName;
		GateClientCallback	gateClientCallback;
	};
}