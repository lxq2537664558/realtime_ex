#pragma once

#include <functional>
#include <vector>
#include <string>
#include <list>

#include "libBaseCommon/ticker.h"
#include "core_common.h"

namespace core
{
	struct SPendingResponseInfo
	{
		base::CTicker	tickTimeout;
		uint64_t		nSessionID;
		uint64_t		nCoroutineID;
		uint64_t		nHolderID;
		std::list<uint64_t>::iterator
						iterHolder;

		int64_t			nBeginTime;	// ∑¢ÀÕ ±º‰
		uint32_t		nToServiceID;
		std::function<void(std::shared_ptr<void>, uint32_t)>
						callback;
	};

	struct	SMessagePacket
	{
		uint8_t		nType;
		uint32_t	nDataSize;
		void*		pData;
	};

#pragma pack(push,1)
	struct gate_forward_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint32_t	nToServiceID;
	};

	struct gate_send_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nToServiceID;
	};

	struct gate_broadcast_cookice
	{
		uint32_t	nToServiceID;
		uint16_t	nSessionCount;
	};

	struct request_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint32_t	nToServiceID;
		uint8_t		nMessageSerializerType;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

	struct response_cookice
	{
		uint64_t	nSessionID;
		uint32_t	nFromServiceID;
		uint32_t	nToServiceID;
		uint32_t	nResult;
		uint8_t		nMessageSerializerType;
		uint16_t	nMessageNameLen;
		char		szMessageName[1];
	};

#pragma pack(pop)
}