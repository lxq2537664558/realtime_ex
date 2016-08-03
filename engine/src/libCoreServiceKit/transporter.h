#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/core_common.h"
#include "libCoreCommon/base_connection.h"

#include "service_base.h"

#include <map>

namespace core
{
	class CTransporter :
		public base::noncopyable
	{
	public:
		CTransporter();
		~CTransporter();

		bool					init();

		bool					invoke(uint16_t nServiceID, const SRequestMessageInfo& sRequestMessageInfo);
		bool					response(uint16_t nServiceID, const SResponseMessageInfo& sResponseMessageInfo);

		bool					forward(uint16_t nServiceID, const SGateForwardMessageInfo& sGateMessageInfo);

		bool					send(uint16_t nServiceID, const SGateMessageInfo& sGateMessageInfo);

		bool					broadcast(uint16_t nServiceID, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo);

		SServiceSessionInfo&	getServiceSessionInfo();

		SResponseWaitInfo*		getResponseWaitInfo(uint64_t nSessionID, bool bErase);
		void					addResponseWaitInfo(uint64_t nSessionID, uint64_t nTraceID);

		uint64_t				genSessionID();

	private:
		void					onRequestMessageTimeout(uint64_t nContext);
		
	private:
		uint64_t									m_nNextSessionID;
		SServiceSessionInfo							m_sServiceSessionInfo;
		std::map<uint64_t, SResponseWaitInfo*>		m_mapResponseWaitInfo;
	};
}