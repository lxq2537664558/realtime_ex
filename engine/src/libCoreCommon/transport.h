#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/core_common.h"

#include "common_base.h"

#include <map>

namespace core
{
	class CTransport :
		public base::noncopyable
	{
	public:
		CTransport();
		~CTransport();

		bool					init();

		bool					call(const std::string& szServiceName, const SRequestMessageInfo& sRequestMessageInfo);
		bool					response(const std::string& szServiceName, const SResponseMessageInfo& sResponseMessageInfo);

		bool					send(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo);

		bool					broadcast(const std::string& szServiceName, const SGateBroadcastMessageInfo& sGateBroadcastMessageInfo);

		bool					route(const std::string& szServiceName, const SGateMessageInfo& sGateMessageInfo);

		void					onConnectRefuse(const std::string& szContext);

		void					sendCacheMessage(const std::string& szServiceName);
		void					delCacheMessage(const std::string& szServiceName);

		SServiceSessionInfo&	getServiceSessionInfo();
		SResponseWaitInfo*		getResponseWaitInfo(uint64_t nSessionID, bool bErase);

	private:
		SMessageCacheInfo*		getMessageCacheInfo(const std::string& szServiceName);
		void					onCheckConnect(uint64_t nContext);
		void					onRequestMessageTimeout(uint64_t nContext);
		uint64_t				genSessionID();
		bool					send(CBaseConnection* pBaseConnection, uint16_t nMessageFormat, const void* pCookice, uint16_t nCookiceSize, const void* pBuf, uint16_t nBufSize);

	private:
		uint64_t									m_nNextSessionID;
		SServiceSessionInfo							m_sServiceSessionInfo;
		CTicker										m_tickCheckConnect;
		std::map<uint64_t, SResponseWaitInfo*>		m_mapResponseWaitInfo;
		std::map<std::string, SMessageCacheInfo>	m_mapMessageCacheInfo;
		std::vector<char>							m_vecBuf;
	};
}