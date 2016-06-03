#pragma once

#include "libCoreCommon/core_common.h"

#include "google/protobuf/message.h"

#include "core_service_kit_define.h"

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
		uint64_t	nSessionID;
		uint32_t	nMessageID;
		void*		pData;
		uint16_t	nSize;
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
		uint32_t			nMessageID;
		std::vector<char>	vecBuf;
		InvokeCallback		callback;
	};

	struct SGateMessageCacheInfo
		: public SMessageCacheHead
	{
		uint32_t			nMessageID;
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
		uint32_t			nMessageID;
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
		eBCT_ConnectionFromService	= 10,
		eBCT_ConnectionToMaster		= 11,
		eBCT_ConnectionToService	= 12,
	};

	struct STraceInfo
	{
		uint64_t	nTraceID;
		std::string	szParentServiceName;
	};
}