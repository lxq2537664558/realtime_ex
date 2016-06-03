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
		virtual ~CCoreConnection();

		bool				init(CBaseConnection* pBaseConnection, uint64_t nID);
		void				send(uint8_t nMessageType, const void* pData, uint16_t nSize);
		void				send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize);
		
		void				setMessageParser(MessageParser& parser);

		uint64_t			getID() const;

		void				shutdown(bool bForce, const std::string& szMsg);

		CBaseConnection*	getBaseConnection() const;

		uint32_t			getSendDataSize() const;
		uint32_t			getRecvDataSize() const;

		const SNetAddr&		getLocalAddr() const;
		const SNetAddr&		getRemoteAddr() const;

		void				onHeartbeat(uint64_t nContext);

	private:
		virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize);
		virtual void		onConnect();
		virtual void		onDisconnect();

		void				onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize);
		void				sendHeartbeat();

	private:
		bool					m_bHeartbeat;
		CTicker*				m_pHeartbeat;
		uint32_t				m_nSendHeartbeatCount;

		MessageParser			m_messageParser;
		uint64_t				m_nID;
		CBaseConnection*		m_pBaseConnection;
		CCoreConnectionMonitor	m_monitor;
	};
}