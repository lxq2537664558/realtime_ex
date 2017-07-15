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
		CMessageDispatcher();
		~CMessageDispatcher();

		bool init(CServiceBaseImpl* pServiceBaseImpl);

		void dispatch(uint64_t nFromSocketID, uint16_t nFromNodeID, uint8_t nMessageType, const google::protobuf::Message* pMessage, const SMCT_RECV_SOCKET_DATA* pContext);

	private:
		CServiceBaseImpl*	m_pServiceBaseImpl;
	};
}