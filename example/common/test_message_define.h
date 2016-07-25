#pragma once
#include "libCoreCommon/core_common.h"

enum 
{
	eClientRequestMsg		= 1000,
	eClientResponseMsg		= 1001,
	eServiceRequestMsg1		= 1002,
	eServiceResponseMsg1	= 1003,
	eServiceRequestMsg2		= 1004,
	eServiceResponseMsg2	= 1005,
	eServiceRequestMsg3		= 1006,
	eServiceResponseMsg3	= 1007,
};

message_begin(SClientRequestMsg, eClientRequestMsg)
	uint32_t nSync;
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SClientResponseMsg, eClientResponseMsg)
	uint32_t nSync;
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceRequestMsg1, eServiceRequestMsg1)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceResponseMsg1, eServiceResponseMsg1)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceRequestMsg2, eServiceRequestMsg2)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceResponseMsg2, eServiceResponseMsg2)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceRequestMsg3, eServiceRequestMsg3)
	uint32_t nID;
	uint64_t nClientTime;
message_end

message_begin(SServiceResponseMsg3, eServiceResponseMsg3)
	uint32_t nID;
	uint64_t nClientTime;
message_end