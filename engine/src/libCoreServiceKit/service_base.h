#pragma once

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

#include <functional>
#include <vector>

namespace core
{
	struct SRequestMessageInfo
	{
		message_header*	pData;
		InvokeCallback	callback;
	};

	struct SResponseMessageInfo
	{
		uint64_t		nSessionID;
		message_header*	pData;
		uint8_t			nResult;
	};

	struct SGateForwardMessageInfo
	{
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

	enum EMessageCacheInfoType
	{
		eMCIT_Request,
		eMCIT_Gate,
		eMCIT_GateBroadcast,
		eMCIT_GateForward,
	};

	struct SMessageCacheHead
	{
		uint8_t		nType;
		uint64_t	nTraceID;
		CTicker		tickTimeout;

		virtual ~SMessageCacheHead() { }
	};

	struct SRequestMessageCacheInfo
		: public SMessageCacheHead
	{
		std::vector<char>	vecBuf;
		InvokeCallback		callback;
	};

	struct SGateMessageCacheInfo
		: public SMessageCacheHead
	{
		uint64_t			nSessionID;
		std::vector<char>	vecBuf;
	};

	struct SGateBroadcastMessageCacheInfo
		: public SMessageCacheHead
	{
		std::vector<uint64_t>	vecSessionID;
		std::vector<char>		vecBuf;
	};

	struct SGateForwardMessageCacheInfo
		: public SMessageCacheHead
	{
		uint64_t			nSessionID;
		std::vector<char>	vecBuf;
	};

	struct SMessageCacheInfo
	{
		uint32_t								nTotalSize;
		bool									bRefuse;
		std::map<uint64_t, SMessageCacheHead*>	mapMessageCacheInfo;
	};

	struct SResponseWaitInfo
	{
		CTicker			tickTimeout;
		uint64_t		nSessionID;
		uint64_t		nTraceID;
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
		GateForwardCallback	gateClientCallback;
	};

	enum EBaseConnectionType
	{
		eBCT_ConnectionToMaster		= 10,
		eBCT_ConnectionService	= 11,
	};

	struct STraceInfo
	{
		uint64_t	nTraceID;
		std::string	szParentServiceName;
	};
}