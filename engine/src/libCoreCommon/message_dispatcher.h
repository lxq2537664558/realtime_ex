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

		void dispatch(uint64_t nFromSocketID, uint16_t nFromNodeID, const SMCT_RECV_SOCKET_DATA* pContext);

	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}