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

		bool					forward(const std::string& szServiceName, const std::string& szMessageName, const SGateForwardMessageInfo& sGateMessageInfo);

		bool					send(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo);

		bool					broadcast(const std::string& szServiceName, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo);

		void					onConnectRefuse(const std::string& szContext);

		void					sendCacheMessage(const std::string& szServiceName);
		void					delCacheMessage(const std::string& szServiceName);

		SServiceSessionInfo&	getServiceSessionInfo();
		SResponseWaitInfo*		getResponseWaitInfo(uint64_t nSessionID, bool bErase);

	private:
		SMessageCacheInfo*		getMessageCacheInfo(const std::string& szServiceName);
		void					onCheckConnect(uint64_t nContext);
		void					onRequestMessageTimeout(uint64_t nContext);
		void					onCacheMessageTimeout(uint64_t nContext);
		uint64_t				genSessionID();
		uint64_t				genCacheID();
		
	private:
		uint64_t									m_nNextSessionID;
		SServiceSessionInfo							m_sServiceSessionInfo;
		CTicker										m_tickCheckConnect;
		std::map<uint64_t, SResponseWaitInfo*>		m_mapResponseWaitInfo;
		std::map<std::string, SMessageCacheInfo>	m_mapMessageCacheInfo;

		uint64_t									m_nCacheID;
		std::vector<char>							m_vecBuf;
	};
}