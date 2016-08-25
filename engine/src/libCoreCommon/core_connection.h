#pragma once
#include "libBaseNetwork/network.h"

#include "core_common.h"
#include "core_connection_monitor.h"

namespace core
{
	class CCoreConnectionMgr;
	class CBaseConnection;
	class CBaseConnectionMgr;
	class CCoreConnection :
		public base::INetConnecterHandler
	{
		friend class CCoreConnectionMgr;
		friend class CBaseConnectionMgr;

	public:
		CCoreConnection();
		~CCoreConnection();

		bool				init(CBaseConnection* pBaseConnection, uint64_t nID, uint32_t nType);
		void				send(uint8_t nMessageType, const void* pData, uint16_t nSize);
		void				send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);
		
		void				setMessageParser(MessageParser& parser);

		uint64_t			getID() const;

		uint32_t			getType() const;

		void				shutdown(bool bForce, const std::string& szMsg);

		CBaseConnection*	getBaseConnection() const;

		uint32_t			getSendDataSize() const;
		uint32_t			getRecvDataSize() const;

		const SNetAddr&		getLocalAddr() const;
		const SNetAddr&		getRemoteAddr() const;

		void				enableHeartbeat(bool bEnable);

		void				setState(uint32_t nState);
		uint32_t			getState() const;

	private:
		uint32_t			onRecv(const char* pData, uint32_t nDataSize);
		void				onConnect();
		void				onDisconnect();

		void				onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);

		void				onHeartbeat(uint64_t nContext);

	private:
		bool					m_bHeartbeat;
		CTicker*				m_pHeartbeat;
		uint32_t				m_nSendHeartbeatCount;

		uint64_t				m_nID;
		uint32_t				m_nType;
		CBaseConnection*		m_pBaseConnection;
		MessageParser			m_messageParser;
		CCoreConnectionMonitor	m_monitor;
	};
}