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

		bool					call(const std::string& szServiceName, const SRequestMessageInfo& sRequestMessageInfo);
		bool					response(const std::string& szServiceName, const SResponseMessageInfo& sResponseMessageInfo);

		bool					forward(const std::string& szServiceName, const SGateForwardMessageInfo& sGateMessageInfo);

		bool					send(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo);

		bool					broadcast(const std::string& szServiceName, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo);

		SServiceSessionInfo&	getServiceSessionInfo();
		SResponseWaitInfo*		getResponseWaitInfo(uint64_t nSessionID, bool bErase);

		uint64_t				genSessionID();

	private:
		void					onRequestMessageTimeout(uint64_t nContext);
		
	private:
		uint64_t									m_nNextSessionID;
		SServiceSessionInfo							m_sServiceSessionInfo;
		std::map<uint64_t, SResponseWaitInfo*>		m_mapResponseWaitInfo;
	};
}