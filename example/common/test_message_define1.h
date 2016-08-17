#pragma once
#include "libCoreCommon/core_common.h"

enum 
{
	eServiceRequestActor1	= 1008,
	eServiceRequestActor2	= 1009,
	eServiceResponseActor2	= 1009,
	eServiceRequestActor3	= 1010,
	eServiceResponseActor3	= 1011,
	eServiceRequestActor4	= 1012,
	eServiceResponseActor4	= 1013,
};

message_begin(CServiceRequestActor1, eServiceRequestActor1)
	uint64_t nActorID;
message_end

message_begin(CServiceRequestActor2, eServiceRequestActor2)
	uint32_t nID;
message_end

message_begin(CServiceResponseActor2, eServiceResponseActor2)
	uint32_t nID;
message_end


message_begin(CServiceRequestActor3, eServiceRequestActor3)
	uint32_t nID;
message_end

message_begin(CServiceResponseActor3, eServiceResponseActor2)
	uint32_t nID;
message_end

message_begin(CServiceRequestActor4, eServiceRequestActor4)
	uint32_t nID;
message_end

message_begin(CServiceResponseActor4, eServiceResponseActor4)
	uint32_t nID;
message_end