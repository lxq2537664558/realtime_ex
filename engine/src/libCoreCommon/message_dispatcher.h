#pragma once

#include "libCoreCommon/core_common.h"

#include "message_command.h"

namespace core
{
	class CCoreService;
	class CMessageDispatcher
	{
	public:
		CMessageDispatcher(CCoreService* pCoreService);
		~CMessageDispatcher();

		void dispatch(uint32_t nFromNodeID, uint8_t nMessageType, const void* pContext);

	private:
		CCoreService*	m_pCoreService;
	};
}