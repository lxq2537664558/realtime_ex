#pragma once

#include "libCoreCommon/core_common.h"

namespace core
{
	class CMessageDispatcher
	{
	public:
		CMessageDispatcher();
		~CMessageDispatcher();

		bool init();
		void dispatch(uint64_t nFromSocketID, uint16_t nFromServiceID, uint8_t nMessageType, const void* pData, uint16_t nSize);
	};
}