#pragma once
#include "libCoreCommon/core_common.h"

enum 
{
	eClientRequestMsg	= 1000,
	eServiceRequestMsg	= 1001,
	eClientResponseMsg	= 1002,
	eServiceResponseMsg	= 1003,
};
message_begin(SClientRequestMsg, eClientRequestMsg)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceRequestMsg, eServiceRequestMsg)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SClientResponseMsg, eClientResponseMsg)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceResponseMsg, eServiceResponseMsg)
	uint32_t nID;
	uint64_t nClientTime;
message_end