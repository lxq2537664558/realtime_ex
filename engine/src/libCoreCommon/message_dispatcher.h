#pragma once

#include "libCoreCommon/core_common.h"

#include "google/protobuf/message.h"

#include "message_command.h"

namespace core
{
	class CServiceBaseImpl;
	class CMessageDispatcher
	{
	public:
		CMessageDispatcher(CServiceBaseImpl* pServiceBaseImpl);
		~CMessageDispatcher();

		void dispatch(uint32_t nFromNodeID, uint8_t nMessageType, const void* pContext);

	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}